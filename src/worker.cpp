#include "protocol.hpp"
#include "profile.hpp"
#include "vendor.hpp"
#include "editing.hpp"
#include <QCoreApplication>
#include <QSocketNotifier>
#include <QTimer>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <sys/prctl.h>
#include <signal.h>

using namespace sogo::direct;
static std::uint64_t monotonicUs(){return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();}
int main(int argc,char** argv){
    int fd=-1;bool probe=false,check=false;
    std::filesystem::path vendor=SOGO_VENDOR_ROOT;
    for(int i=1;i<argc;++i){
        std::string arg=argv[i];
        if(arg=="--fd"&&i+1<argc)fd=std::stoi(argv[++i]);
        else if(arg=="--vendor-root"&&i+1<argc)vendor=argv[++i];
        else if(arg=="--probe")probe=true;
        else if(arg=="--check")check=true;
        else {std::cerr<<"invalid engine argument\n";return 2;}
    }
    if(!probe&&!check&&fd<0)return 2;
    umask(0077);
    if(fd>=0){const auto parent=getppid();prctl(PR_SET_PDEATHSIG,SIGTERM);if(getppid()!=parent)return 1;}
    try{
        if(check){Vendor::verify(vendor);std::cout<<"{\"vendor_hashes\":true,\"vendor_code_executed\":false}\n";return 0;}
        const auto root=dataRoot();const auto profile=prepareProfile(root);
        // Keep this descriptor open until process exit, including the vendor's
        // atexit callbacks. An additional worker or probe cannot write the same
        // learning files concurrently.
        const auto profileLock=acquireProfileLock(root);(void)profileLock;
        // Private D-Bus and GUI applications are not needed by this endpoint.
        QCoreApplication application(argc,argv);
        Vendor engine(vendor,profile);
        if(probe){
            for(char key:std::string("nihao")){
                engine.key(virtualKey(key),0,false);engine.key(virtualKey(key),0,true);
            }
            auto result=engine.snapshot();
            const bool candidate=std::find(result.candidates.begin(),result.candidates.end(),"你好")!=result.candidates.end();
            engine.key(32,0,false);auto committed=engine.snapshot();engine.key(32,0,true);
            std::cout<<"{\"layout\":19,\"nihao_candidate\":"<<(candidate?"true":"false")
                     <<",\"nihao_commit\":"<<(committed.result=="你好"?"true":"false")<<"}\n";
            return candidate&&committed.result=="你好"?0:1;
        }
        if(!send(fd,packet(Command::Ready)))return 1;
        std::uint64_t generation=0,context=0;Editing editing;
        QSocketNotifier notifier(fd,QSocketNotifier::Read);
        QObject::connect(&notifier,&QSocketNotifier::activated,&application,[&](){
            for(unsigned i=0;i<32;++i){
                Message message;const auto status=receive(fd,message);
                if(status==0)return;
                if(status<0){application.quit();return;}
                const auto& h=message.header;const auto command=static_cast<Command>(h.type);
                try{
                    if(command==Command::Stop){application.quit();return;}
                    if(command==Command::Focus){engine.reset();editing.reset();generation=h.generation;context=h.context;continue;}
                    if(command==Command::Blur){engine.reset();editing.reset();generation=0;context=0;continue;}
                    if(h.generation!=generation||h.context!=context)continue;
                    const auto begin=monotonicUs();auto outcome=editing.process(engine,h);
                    auto reply=h;reply.type=static_cast<std::uint32_t>(Command::Reply);
                    reply.handled=outcome.flags;
                    reply.engineUs=monotonicUs()-begin;
                    if(!send(fd,reply,encode(outcome.value))){application.quit();return;}
                }catch(const std::exception&){send(fd,packet(Command::Fault));application.quit();return;}
            }
        });
        return application.exec();
    }catch(const std::exception& error){
        // Error messages contain interface/resource names, never input text.
        if(fd>=0)send(fd,packet(Command::Fault));
        std::cerr<<"sogo-fc5-direct: "<<error.what()<<'\n';return 1;
    }
}
