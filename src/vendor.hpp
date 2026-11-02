#pragma once
#include "keys.hpp"
#include "learning.hpp"
#include "result_buffers.hpp"
#include "shell_view.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <dlfcn.h>
#include <filesystem>
#include <memory>
#include <string>

namespace sogo::direct {
class Vendor {
public:
    static void verify(const std::filesystem::path& root) {
        for(const auto& [name,hash]:std::array<std::pair<const char*,const char*>,3>{{
            {"libSogouShell.so","6fc1db35ba40e687621d138f142cf971d98d904f61e48767aef15b03ab520f50"},
            {"libSogouIme.so","e821f0de7193ced47ebaa592827f85c31da861075e98817bdea0bda6828665b7"},
            {"libSogouWbShell.so","3488f903adc9c46f78b620537350c055cb7d5448166bf8229285b8964e1e0ee4"}}}) {
            QFile file(QString::fromStdString((root/"lib"/name).string()));
            if(!file.open(QIODevice::ReadOnly))throw std::runtime_error("missing vendor library");
            QCryptographicHash digest(QCryptographicHash::Sha256);
            if(!digest.addData(&file)||digest.result().toHex()!=hash)throw std::runtime_error("vendor library hash mismatch");
        }
    }
    Vendor(const std::filesystem::path& root,const std::filesystem::path& profile) {
        verify(root);
        // RTLD_LOCAL keeps the original C++ and bundled utility symbols inside
        // the engine process; the desktop never loads these libraries.
        library_=dlopen((root/"lib/libSogouIme.so").c_str(),RTLD_NOW|RTLD_LOCAL);
        if(!library_){const char* message=dlerror();throw std::runtime_error(message?message:"cannot load vendor engine");}
        auto environment=symbol<void*(*)()>("GetSGAuthEnv")();
        auto initialize=symbol<bool(*)(void*,const char*,const char*)>("_ZN8n_sgAuth11t_stAuthEnv4InitEPKcS2_");
        if(!initialize(environment,root.c_str(),profile.c_str()))throw std::runtime_error("vendor environment initialization failed");
        if(!symbol<bool(*)(const void*)>("_ZNK8n_sgAuth11t_stAuthEnv11IsAuthValidEv")(environment))
            throw std::runtime_error("vendor resource authorization failed");
        shell_=symbol<void*(*)()>("GetSogouShell")();
        release_=symbol<void(*)(void*)>("Release");
        if(!shell_)throw std::runtime_error("vendor shell factory failed");
        view_=std::make_unique<fc4::ShellView>(shell_);
        view_->setAuthEnvironment(environment);
        const auto dictionary=root/"share/shell/dict",user=profile/"dict",conf=profile/"conf",resources=root/"share/resources";
        if(!view_->initialize(dictionary.c_str(),user.c_str(),conf.c_str(),resources.c_str(),root.c_str()))
            throw std::runtime_error("vendor dictionary initialization failed");
        if(!view_->activate(19))throw std::runtime_error("vendor PC pinyin activation failed");
        keyboard_=symbol<void*(*)()>("_ZN17t_keyboardManager11GetInstanceEv")();
        keyboardState_=symbol<unsigned char*(*)(void*)>("_ZN17t_keyboardManager16GetKeyboardStateEv");
        if(!keyboard_||!keyboardState_(keyboard_))throw std::runtime_error("vendor keyboard state unavailable");
        Dl_info image{};
        auto factory=symbol<void*(*)()>("GetSogouShell");
        void* address=nullptr;std::memcpy(&address,&factory,sizeof(address));
        if(!dladdr(address,&image)||!image.dli_fbase)throw std::runtime_error("vendor image base unavailable");
        imageBase_=static_cast<unsigned char*>(image.dli_fbase);
        void* table=nullptr;std::memcpy(&table,shell_,sizeof(table));
        if(table!=imageBase_+0x9ec750)throw std::runtime_error("unsupported vendor shell vtable");
        void*(*getEnvironment)()=nullptr;
        address=imageBase_+0x422af6;std::memcpy(&getEnvironment,&address,sizeof(getEnvironment));
        auto* inputEnvironment=getEnvironment();
        auto* keypadEntry=*symbol<void**>("BOOL_PadNumAsSelect");
        if(!inputEnvironment||!keypadEntry)throw std::runtime_error("missing vendor keypad setting");
        symbol<void(*)(void*,const void*,bool)>("_ZN5t_env12SetValueBoolEPK10t_envEntryIbEb")(inputEnvironment,keypadEntry,false);
        if(symbol<bool(*)(const void*,const void*)>("_ZNK5t_env12GetValueBoolEPK10t_envEntryIbE")(inputEnvironment,keypadEntry))
            throw std::runtime_error("vendor keypad setting was not applied");
        // Shell refresh copies core slot 25 into the PC UTF-8 result. Clearing
        // just that UTF-8 copy leaves the core result available for replay.
        address=imageBase_+0x7cd5c;
        std::memcpy(&assign_,&address,sizeof(assign_));
        const auto requireTable=[&](void* object,std::size_t offset){
            void* actual=nullptr;
            if(object)std::memcpy(&actual,object,sizeof(actual));
            if(actual!=imageBase_+offset)throw std::runtime_error("unsupported vendor result interface");
        };
        void* pc=nullptr;std::memcpy(&pc,static_cast<unsigned char*>(shell_)+0x38,sizeof(pc));
        requireTable(pc,0x9ecc10);
        void* core=nullptr;std::memcpy(&core,static_cast<unsigned char*>(pc)+0x130,sizeof(core));
        requireTable(core,0x9fc310);
        void* object=view_->resultObject();requireTable(object,0x9ec1f0);
        ResultBuffers::GetCore get=nullptr;
        address=imageBase_+0x410d3e;std::memcpy(&get,&address,sizeof(get));
        results_=std::make_unique<ResultBuffers>(core,get,object,assign_);
        learning_=std::make_unique<NativeLearning>(native<NativeLearning::Factory>(0x237d60),
            native<NativeLearning::Save>(0x10d880),native<NativeLearning::Save>(0x10d960));
        reset();
    }
    ~Vendor(){
        if(shell_&&release_)release_(shell_);
        // The original shell registers learning-data cleanup with the dynamic
        // loader. Unloading follows the original ownership of those singletons.
        if(library_)dlclose(library_);
    }
    Vendor(const Vendor&)=delete;Vendor& operator=(const Vendor&)=delete;
    void saveLearning(bool sync){learning_->save(sync);}
    void reset(){view_->clear();std::memset(keyboardState_(keyboard_),0,256);}
    void begin(){results_->clear();}
    bool key(std::uint8_t vk,std::uint32_t state,bool release){
        begin();modifiers(keyboardState_(keyboard_),engineState(vk,state));return view_->insertChar(shellKey(vk,release));
    }
    void select(std::int32_t index){begin();view_->select(index);}
    bool previous(){begin();return view_->pageUp();}
    bool next(){begin();return view_->pageDown();}
    fc4::ShellView::Snapshot snapshot()const{return view_->snapshot();}
private:
    template<class Function> Function native(std::size_t offset)const{
        void* address=imageBase_+offset;Function function;
        static_assert(sizeof(function)==sizeof(address));std::memcpy(&function,&address,sizeof(function));return function;
    }
    template<class Function> Function symbol(const char* name){
        dlerror();void* address=dlsym(library_,name);const char* error=dlerror();
        if(error||!address)throw std::runtime_error(std::string("missing vendor interface: ")+name);
        Function function;static_assert(sizeof(function)==sizeof(address));std::memcpy(&function,&address,sizeof(function));return function;
    }
    void* library_=nullptr;void* shell_=nullptr;void* keyboard_=nullptr;
    unsigned char* imageBase_=nullptr;
    void(*release_)(void*)=nullptr;void*(*assign_)(void*,const char*)=nullptr;
    unsigned char*(*keyboardState_)(void*)=nullptr;
    std::unique_ptr<fc4::ShellView> view_;
    std::unique_ptr<ResultBuffers> results_;
    std::unique_ptr<NativeLearning> learning_;
};
}
