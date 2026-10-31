#include <cassert>
#include <new>
#include <initializer_list>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
extern "C" void sogo_bad_array_length() asm("__cxa_throw_bad_array_new_length");
extern "C" void sogo_forbidden_window_c1(void*,void*,void*) asm("_ZN6n_sgxx10MainWindowC1EP7QWidgetPNS_16t_wndTopImplBaseE");
extern "C" void sogo_forbidden_window_c2(void*,void*,void*) asm("_ZN6n_sgxx10MainWindowC2EP7QWidgetPNS_16t_wndTopImplBaseE");
extern "C" int XTestFakeKeyEvent(void*,unsigned int,int,unsigned long);
extern "C" int XTestFakeButtonEvent(void*,unsigned int,int,unsigned long);
extern "C" int XGrabKeyboard(void*,unsigned long,int,int,int,unsigned long);
int main(){
    bool caught=false;try{sogo_bad_array_length();}catch(const std::bad_array_new_length&){caught=true;}assert(caught);
    assert(XTestFakeKeyEvent(nullptr,0,0,0)==0);
    assert(XTestFakeButtonEvent(nullptr,0,0,0)==0);
    assert(XGrabKeyboard(nullptr,0,0,0,0,0)==1);
    for(auto function:{sogo_forbidden_window_c1,sogo_forbidden_window_c2}){
        const auto child=fork();assert(child>=0);
        if(!child){rlimit limit{0,0};setrlimit(RLIMIT_CORE,&limit);function(nullptr,nullptr,nullptr);_exit(1);}
        int status=0;assert(waitpid(child,&status,0)==child);assert(WIFSIGNALED(status)&&WTERMSIG(status)==SIGABRT);
    }
}
