#include "presentation.hpp"
#include <fcitx/inputcontextmanager.h>
#include <chrono>
#include <iostream>

using namespace fcitx;
using sogo::fc4::ShellView;
class Client final:public InputContext {
public:
    explicit Client(InputContextManager& manager):InputContext(manager,"benchmark"){created();setCapabilityFlags(CapabilityFlag::Preedit);}
    ~Client()override{destroy();}
    const char* frontend()const override{return "benchmark";}
    std::uint64_t commits=0,preedits=0;
protected:
    void commitStringImpl(const std::string&)override{++commits;}
    void deleteSurroundingTextImpl(int,unsigned)override{}
    void forwardKeyImpl(const ForwardKeyEvent&)override{}
    void updatePreeditImpl()override{++preedits;}
};
static void unconditional(Client& client,const ShellView::Snapshot& value){
    sogo::direct::validate(value);
    if(!value.result.empty())client.commitString(value.result);
    Text preedit(value.composition,TextFormatFlag::Underline);
    preedit.setCursor(value.composition.size());
    client.inputPanel().setPreedit(preedit);client.inputPanel().setClientPreedit(preedit);
    if(value.candidates.empty())client.inputPanel().setCandidateList(nullptr);
    else client.inputPanel().setCandidateList(std::make_unique<sogo::direct::Page>(value.candidates,0,
        [](int,InputContext*){},[](bool){}));
    client.updatePreedit();client.updateUserInterface(UserInterfaceComponent::InputPanel);
}
struct Measurement {std::uint64_t us,preedits,candidates,commits;};
static Measurement run(bool cached,const std::vector<ShellView::Snapshot>& events,unsigned cycles){
    InputContextManager manager;Client client(manager);sogo::direct::Presentation presentation;
    std::uint64_t candidates=0;
    auto start=std::chrono::steady_clock::now();
    for(unsigned i=0;i<cycles;++i)for(const auto& event:events){
        if(cached){auto changed=presentation.present(&client,event,0,[](int,InputContext*){},[](bool){});candidates+=changed.candidates;}
        else {unconditional(client,event);++candidates;}
    }
    auto elapsed=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count();
    return {static_cast<std::uint64_t>(elapsed),client.preedits,candidates,client.commits};
}
int main(){
    std::vector<ShellView::Snapshot> events;
    for(std::size_t i=1;i<=5;++i){
        ShellView::Snapshot value;value.composition=std::string("nihao").substr(0,i);
        value.candidates={"你好"+std::to_string(i),"拟好","你号","泥好","倪好","你浩"};
        events.push_back(value);events.push_back(value);
    }
    ShellView::Snapshot commit;commit.result="你好";events.push_back(commit);events.emplace_back();
    constexpr unsigned cycles=10000,rounds=7;
    std::vector<std::uint64_t> baseline,optimized;Measurement before{},after{};
    run(false,events,100);run(true,events,100);
    for(unsigned i=0;i<rounds;++i){
        if(i%2){after=run(true,events,cycles);before=run(false,events,cycles);}
        else {before=run(false,events,cycles);after=run(true,events,cycles);}
        if(before.commits!=cycles||after.commits!=cycles)return 1;
        baseline.push_back(before.us);optimized.push_back(after.us);
    }
    std::sort(baseline.begin(),baseline.end());std::sort(optimized.begin(),optimized.end());
    std::cout<<"{\"events_per_round\":"<<events.size()*cycles<<",\"rounds\":"<<rounds
        <<",\"unconditional_median_us\":"<<baseline[rounds/2]<<",\"cached_median_us\":"<<optimized[rounds/2]
        <<",\"unconditional_preedit_calls\":"<<before.preedits<<",\"cached_preedit_calls\":"<<after.preedits
        <<",\"unconditional_candidate_updates\":"<<before.candidates<<",\"cached_candidate_updates\":"<<after.candidates
        <<",\"commits_per_round\":"<<after.commits<<",\"desktop_rendering_measured\":false,\"vendor_executed\":false}\n";
}
