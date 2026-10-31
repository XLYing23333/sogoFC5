#include "keys.hpp"
#include "protocol.hpp"
#include "presentation.hpp"
#include "scheduling.hpp"
#include "latency.hpp"
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/action.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>
#include <fcitx/statusarea.h>
#include <fcitx/userinterfacemanager.h>
#include <fcitx-config/option.h>
#include <fcitx-utils/event.h>
#include <fcitx-utils/utf8.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <spawn.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <set>
extern char** environ;
using namespace fcitx;
using namespace sogo::direct;

namespace {
class Engine;
class Engine final:public InputMethodEngine {
public:
    explicit Engine(Instance* instance):instance_(instance){
        palette_.setShortText("Dark Mode / WeLight Mode");palette_.setIcon("preferences-desktop-theme");
        palette_.registerAction("sogo-direct-palette",&instance_->userInterfaceManager());
        paletteConnection_=palette_.connect<SimpleAction::Activated>([this](InputContext*){changePalette();});
        maintenanceAt_=now(CLOCK_MONOTONIC)+MaintenanceUs;
        timer_=instance_->eventLoop().addTimeEvent(CLOCK_MONOTONIC,maintenanceAt_,1000,
            [this](EventSourceTime*,std::uint64_t){++timerWakeups_;tick();armTimer();return true;});
        start();armTimer();
    }
    ~Engine()override{
        timer_.reset();io_.reset();
        if(fd_>=0){send(fd_,packet(Command::Stop));close(fd_);}
        // The engine exits when its parent disappears. Stop normally first so
        // vendor cleanup can save learning data; no desktop process is killed.
    }
    void activate(const InputMethodEntry&,InputContextEvent& event)override{
        context_=event.inputContext()->watch();active_=true;page_=0;composing_=false;
        routes_.reset();owned_.clear();pending_.focus(++contextId_);clearPanel();
        event.inputContext()->statusArea().addAction(StatusGroup::InputMethod,&palette_);
        if(ready_)focus();else if(child_<=0)start();
    }
    void deactivate(const InputMethodEntry&,InputContextEvent&)override{
        pending_.focus(pending_.context);routes_.reset();owned_.clear();active_=false;composing_=false;
        if(fd_>=0)send(fd_,packet(Command::Blur));
        clearPanel();
    }
    void reset(const InputMethodEntry&,InputContextEvent&)override{
        pending_.focus(pending_.context);owned_.clear();routes_.reset();page_=0;composing_=false;
        clearPanel();if(active_&&ready_)focus();
    }
    void keyEvent(const InputMethodEntry&,KeyEvent& event)override{
        receive();
        const bool allow=!(event.rawKey().sym()>=0xff00&&!composing_&&!pending_.mayCompose());
        if(!routes_.native(event.rawKey().sym(),event.rawKey().states(),event.rawKey().code(),event.isRelease(),allow))return;
        if(!active_)return;
        if(!ready_){failure("engine-not-ready",false);return;}
        if(pending_.expired(now(CLOCK_MONOTONIC))){failure("key-deadline");return;}
        auto h=packet(Command::Key);h.sym=event.rawKey().sym();h.state=event.rawKey().states();
        h.code=event.rawKey().code();h.release=event.isRelease();h.time=event.time();
        event.filterAndAccept();enqueue(h);
    }
private:
    static std::uint64_t identity(const Header& h){return h.code?(std::uint64_t(1)<<32)|h.code:virtualKey(h.sym);}
    void enqueue(Header h){
        if(!pending_.push(h,now(CLOCK_MONOTONIC))){failure("queue-overflow");forward(h);return;}
        maxPending_=std::max(maxPending_,pending_.keys.size());
        if(!send(fd_,h))failure("request-write");
        armTimer();
    }
    void armTimer(){
        timer_->setTime(nextWake(now(CLOCK_MONOTONIC),pending_,child_>0,ready_,startedAt_,retryAt_,maintenanceAt_));
        timer_->setOneShot();
    }
    void focus(){if(!send(fd_,packet(Command::Focus,pending_.generation,pending_.context)))failure("focus-write");}
    void clearPanel(){
        presentation_.reset();
        if(auto* ic=context_.get()){ic->inputPanel().reset();ic->updatePreedit();ic->updateUserInterface(UserInterfaceComponent::InputPanel);}
    }
    void forward(const Header& h){
        if(auto* ic=context_.get();ic&&ic->hasFocus()){
            ic->forwardKey(Key(static_cast<KeySym>(h.sym),KeyStates(applicationState(h.state)),h.code),h.release,h.time);++forwarded_;
        }
    }
    void complete(const Pending& request,const Header& reply,const sogo::fc4::ShellView::Snapshot& value){
        auto* ic=context_.get();if(!active_||!ic||!ic->hasFocus())return;
        validate(value);
        if(!value.result.empty()){++commits_;page_=0;}
        if(reply.handled&ReplyPageReset)page_=0;
        else if(reply.handled&ReplyPageNext)++page_;
        else if(reply.handled&ReplyPagePrevious)page_=std::max(0,page_-1);
        if(request.key.type==static_cast<std::uint32_t>(Command::Key)){
            const auto id=identity(request.key);
            if(!request.key.release){
                const bool owned=(reply.handled&ReplyHandled)||!value.result.empty()||!value.composition.empty();
                if(owned)owned_.insert(id);else {owned_.erase(id);forward(request.key);}
            }else {
                const bool owned=owned_.erase(id)>0;
                if(!owned&&!(reply.handled&ReplyHandled)&&value.result.empty())forward(request.key);
            }
        }
        composing_=!value.composition.empty();
        const auto generation=pending_.generation,id=pending_.context;
        auto valid=[this,generation,id](InputContext* target){return active_&&ready_&&generation==pending_.generation&&id==pending_.context&&target==context_.get()&&target->hasFocus();};
        const auto begin=now(CLOCK_MONOTONIC);
        const auto changed=presentation_.present(ic,value,page_,
            [this,valid](int index,InputContext* target){if(valid(target)){auto h=packet(Command::Select);h.sym=index;enqueue(h);}},
            [this,valid](bool next){auto* target=context_.get();if(target&&valid(target)){
                enqueue(packet(next?Command::Next:Command::Previous));}});
        presentationLatency_.add(now(CLOCK_MONOTONIC)-begin);
        preeditUpdates_+=changed.preedit;candidateUpdates_+=changed.candidates;uiUpdates_+=changed.ui;
    }
    void receive(){
        if(receiving_||fd_<0)return;
        struct Guard{Engine* engine;Guard(Engine* v):engine(v){engine->receiving_=true;}
            ~Guard(){engine->receiving_=false;engine->armTimer();}}guard(this);
        for(unsigned i=0;i<256&&fd_>=0;++i){
            Message message;auto status=sogo::direct::receive(fd_,message);if(status==0)return;
            if(status<0){failure("engine-disconnected");return;}
            auto command=static_cast<Command>(message.header.type);
            if(command==Command::Ready){ready_=true;backoff_=1000000;if(active_)focus();continue;}
            if(command==Command::Fault){failure("engine-fault");return;}
            if(command!=Command::Reply){failure("invalid-engine-message");return;}
            Pending request;const auto match=pending_.take(message.header,request);
            if(match==PendingQueue::Match::Stale){++stale_;continue;}
            if(match==PendingQueue::Match::Invalid){failure("invalid-engine-sequence");return;}
            const auto time=now(CLOCK_MONOTONIC);
            if(time-request.queued>=DeadlineUs){pending_.keys.push_front(request);failure("key-deadline");return;}
            try{complete(request,message.header,decode(message.body));}
            catch(const std::exception&){pending_.keys.push_front(request);failure("invalid-engine-result");return;}
            ++completed_;deliveryLatency_.add(time-request.queued);engineLatency_.add(message.header.engineUs);
            totalLatency_.add(now(CLOCK_MONOTONIC)-request.queued);
        }
    }
    void failure(const char* reason,bool stopWorker=true){
        lastFault_=reason;++faults_;
        // Each outstanding key belongs to exactly one ordered request. No
        // already acknowledged request is replayed during fault recovery.
        for(const auto& item:pending_.keys)if(item.key.type==static_cast<std::uint32_t>(Command::Key))forward(item.key);
        pending_.focus(pending_.context);routes_.reset();owned_.clear();clearPanel();
        const bool active=active_;active_=false;ready_=false;composing_=false;
        // An input attempt during startup leaves initialization running.
        // Only a transport or engine fault terminates the owned worker.
        if(stopWorker){
            if(io_)io_->setEnabled(false);
            if(fd_>=0){close(fd_);fd_=-1;}
            if(child_>0)kill(-child_,SIGKILL);
            retryAt_=now(CLOCK_MONOTONIC)+backoff_;backoff_=std::min<std::uint64_t>(backoff_*2,30000000);
        }
        if(active){if(auto* ic=context_.get();ic&&ic->hasFocus())instance_->setCurrentInputMethod(ic,"keyboard-us",true);}
        publish(true);
    }
    void start(){
        if(child_>0)return;
        io_.reset();
        int sockets[2];if(socketpair(AF_UNIX,SOCK_SEQPACKET|SOCK_NONBLOCK|SOCK_CLOEXEC,0,sockets)){
            retryAt_=now(CLOCK_MONOTONIC)+1000000;return;
        }
        auto inherited=fcntl(sockets[1],F_DUPFD_CLOEXEC,10);
        if(inherited<0){close(sockets[0]);close(sockets[1]);return;}
        posix_spawn_file_actions_t actions;posix_spawn_file_actions_init(&actions);
        posix_spawn_file_actions_adddup2(&actions,inherited,3);posix_spawn_file_actions_addclose(&actions,inherited);
        posix_spawn_file_actions_addopen(&actions,0,"/dev/null",O_RDONLY,0);
        posix_spawn_file_actions_addopen(&actions,1,"/dev/null",O_WRONLY,0);
        posix_spawn_file_actions_addopen(&actions,2,"/dev/null",O_WRONLY,0);
        posix_spawnattr_t attributes;posix_spawnattr_init(&attributes);
        posix_spawnattr_setflags(&attributes,POSIX_SPAWN_SETPGROUP);posix_spawnattr_setpgroup(&attributes,0);
        const char* executable=SOGO_ENGINE_PATH;
        char* args[]={const_cast<char*>(executable),const_cast<char*>("--fd"),const_cast<char*>("3"),nullptr};
        const auto error=posix_spawn(&child_,executable,&actions,&attributes,args,environ);
        posix_spawn_file_actions_destroy(&actions);posix_spawnattr_destroy(&attributes);
        close(inherited);close(sockets[1]);
        if(error){child_=-1;close(sockets[0]);retryAt_=now(CLOCK_MONOTONIC)+1000000;lastFault_="engine-spawn";return;}
        fd_=sockets[0];ready_=false;startedAt_=now(CLOCK_MONOTONIC);
        io_=instance_->eventLoop().addIOEvent(fd_,IOEventFlags{IOEventFlag::In,IOEventFlag::Hup,IOEventFlag::Err},
            [this](EventSourceIO*,int,IOEventFlags){receive();return true;});
    }
    void tick(){
        receive();const auto time=now(CLOCK_MONOTONIC);
        if(time>=maintenanceAt_)maintenanceAt_=time+MaintenanceUs;
        if(pending_.expired(time))failure("key-deadline");
        if(child_>0){
            if(waitpid(child_,nullptr,WNOHANG)==child_){child_=-1;if(fd_>=0)failure("engine-exit");}
            else if(fd_>=0&&!ready_&&time-startedAt_>=10000000)failure("engine-startup-deadline");
        }
        if(child_<=0&&time>=retryAt_)start();
        publish(false);
    }
    void changePalette(){
        auto* classic=instance_->addonManager().addon("classicui",true);
        if(!classic||!classic->getConfig())return;
        RawConfig config;classic->getConfig()->save(config);
        const auto theme=config.get("Theme");
        const bool dark=theme&&theme->value().find("dark")!=std::string::npos;
        config["Theme"].setValue(dark?"sogo-direct-welight-mode":"sogo-direct-dark-mode");
        config["UseDarkTheme"].setValue("False");
        // Classic UI owns this setting. This call never resets an input
        // context, reloads the engine, or changes the active input method.
        classic->setConfig(config);
    }
    void publish(bool force){
        const auto time=now(CLOCK_MONOTONIC);if(!force&&time-lastPublish_<1000000)return;lastPublish_=time;
        const auto runtime=std::getenv("XDG_RUNTIME_DIR");if(!runtime||!*runtime)return;
        const auto directory=std::filesystem::path(runtime)/"sogo-fc5-direct";
        std::error_code error;std::filesystem::create_directory(directory,error);
        struct stat info{};if(lstat(directory.c_str(),&info)||!S_ISDIR(info.st_mode)||info.st_uid!=getuid())return;
        chmod(directory.c_str(),0700);
        auto temporary=(directory/"status.json.XXXXXX").string();int fd=mkstemp(temporary.data());if(fd<0)return;
        const auto json="{\"version\":\"" SOGO_VERSION "\",\"backend\":\"official-fc4-shell\",\"ready\":"+
            std::string(ready_?"true":"false")+",\"active\":"+(active_?"true":"false")+",\"pid\":"+std::to_string(child_)+
            ",\"completed\":"+std::to_string(completed_)+",\"max_pending\":"+std::to_string(maxPending_)+
            ",\"mean_total_us\":"+std::to_string(totalLatency_.mean())+",\"max_total_us\":"+std::to_string(totalLatency_.maximum())+
            ",\"mean_delivery_us\":"+std::to_string(deliveryLatency_.mean())+
            ",\"mean_engine_us\":"+std::to_string(engineLatency_.mean())+",\"max_engine_us\":"+std::to_string(engineLatency_.maximum())+
            ",\"p95_total_us_upper\":"+std::to_string(totalLatency_.percentileUpper(95))+
            ",\"p99_total_us_upper\":"+std::to_string(totalLatency_.percentileUpper(99))+
            ",\"p95_engine_us_upper\":"+std::to_string(engineLatency_.percentileUpper(95))+
            ",\"mean_present_us\":"+std::to_string(presentationLatency_.mean())+
            ",\"max_present_us\":"+std::to_string(presentationLatency_.maximum())+
            ",\"ui_updates\":"+std::to_string(uiUpdates_)+",\"preedit_updates\":"+std::to_string(preeditUpdates_)+
            ",\"candidate_updates\":"+std::to_string(candidateUpdates_)+",\"timer_wakeups\":"+std::to_string(timerWakeups_)+
            ",\"commits\":"+std::to_string(commits_)+
            ",\"forwarded\":"+std::to_string(forwarded_)+",\"stale\":"+std::to_string(stale_)+",\"faults\":"+
            std::to_string(faults_)+",\"last_fault\":\""+lastFault_+"\"}\n";
        const auto count=write(fd,json.data(),json.size());close(fd);
        if(count!=static_cast<ssize_t>(json.size())||rename(temporary.c_str(),(directory/"status.json").c_str()))unlink(temporary.c_str());
    }
    Instance* instance_;TrackableObjectReference<InputContext> context_;PendingQueue pending_;KeyRoutes routes_;
    std::set<std::uint64_t> owned_;bool active_=false,ready_=false,receiving_=false,composing_=false;
    int fd_=-1,page_=0;pid_t child_=-1;
    std::uint64_t contextId_=0,startedAt_=0,retryAt_=0,backoff_=1000000,lastPublish_=0,maintenanceAt_=0;
    Presentation presentation_;Latency totalLatency_,deliveryLatency_,engineLatency_,presentationLatency_;
    std::uint64_t completed_=0,commits_=0,forwarded_=0,stale_=0,faults_=0;
    std::uint64_t preeditUpdates_=0,candidateUpdates_=0,uiUpdates_=0,timerWakeups_=0;
    std::size_t maxPending_=0;std::string lastFault_;
    SimpleAction palette_;ScopedConnection paletteConnection_;
    std::unique_ptr<EventSourceTime> timer_;std::unique_ptr<EventSourceIO> io_;
};
class Factory:public AddonFactory {
    AddonInstance* create(AddonManager* manager)override{return new Engine(manager->instance());}
};
}
FCITX_ADDON_FACTORY(Factory)
