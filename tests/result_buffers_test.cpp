#include "editing.hpp"
#include "result_buffers.hpp"
#include <algorithm>
#include <cassert>
#include <cwchar>

using namespace sogo::direct;
class Backend {
public:
    explicit Backend(bool both=true):both_(both),buffers_(&core_,get,&value_.result,assign){}
    void begin(){if(both_)buffers_.clear();else value_.result.clear();}
    bool key(unsigned char vk,std::uint32_t,bool release){
        begin();
        if(release){refresh();return false;}
        if(vk==8){if(!value_.composition.empty())value_.composition.pop_back();}
        else if(vk=='1')commit(false);
        else if(vk=='2')commit(true);
        else if(vk>='A'&&vk<='Z')value_.composition+=char(vk-'A'+'a');
        refresh();return true;
    }
    void select(std::int32_t index){begin();commit(index==1);refresh();}
    bool next(){begin();page_=1;refresh();return true;}
    bool previous(){begin();page_=0;refresh();return true;}
    void reset(){begin();value_={};page_=0;}
    auto snapshot()const{return value_;}
private:
    struct Core {wchar_t result[8]{};} core_;
    static bool get(void* object,wchar_t** output){*output=static_cast<Core*>(object)->result;return true;}
    static void* assign(void* object,const char* text){*static_cast<std::string*>(object)=text;return object;}
    void commit(bool all){
        if(value_.composition.empty())return;
        std::wcscpy(core_.result,L"甲词");
        if(all)value_.composition.clear();else value_.composition.erase(0,2);
    }
    void refresh(){
        // The converted view is rebuilt from persistent core output on every
        // refresh, including releases and events with no new conversion.
        value_.result=core_.result[0]?"甲词":"";
        value_.candidates=value_.composition.empty()?std::vector<std::string>{}:
            std::vector<std::string>{"甲词","page-"+std::to_string(page_)};
    }
    bool both_;ResultBuffers buffers_;int page_=0;
    sogo::fc4::ShellView::Snapshot value_;
};
Header key(unsigned sym,bool release=false,std::uint32_t state=0){auto h=packet(Command::Key);h.sym=sym;h.release=release;h.state=state;return h;}

unsigned continuation(bool both){
    Backend backend(both);Editing editing;
    for(unsigned sym:std::string("abcdef")){
        editing.process(backend,key(sym));editing.process(backend,key(sym,true));
    }
    auto out=editing.process(backend,packet(Command::Select));
    assert(out.value.result=="甲词"&&out.value.composition=="cdef");
    unsigned repeated=0;
    for(unsigned sym:std::string("xyzxyz"))for(bool release:{false,true}){
        out=editing.process(backend,key(sym,release));
        repeated+=!out.value.result.empty();
        assert(out.flags&ReplyHandled);
    }
    assert(out.value.composition=="cdefxyzxyz");
    return repeated;
}
int main(){
    // A display-only reset reproduces repeated commits; the same event stream
    // with the production buffer reset retains the suffix without replay.
    assert(continuation(false)==12);
    assert(continuation(true)==0);
    for(bool mouse:{false,true})for(bool releases:{false,true}){
        Backend backend;Editing editing;
        auto send=[&](Header h){return editing.process(backend,h);};
        for(int i=0;i<96;++i){send(key('a'));send(key('a',true));}
        auto select=packet(Command::Select);
        auto out=send(mouse?select:key('1'));
        assert(out.value.result=="甲词"&&out.value.composition.size()==94);
        if(!mouse)assert(send(key('1',true)).value.result.empty());
        for(int i=0;i<160;++i){
            out=send(key('z',false,i?RepeatState:0));
            assert(out.value.result.empty()&&(out.flags&ReplyHandled));
            if(releases)assert(send(key('z',true)).value.result.empty());
        }
        assert(out.value.composition.size()==254);
        for(auto h:{key('='),key('=',true),key('-'),key('-',true),packet(Command::Next),packet(Command::Previous)})
            assert(send(h).value.result.empty());
        out=send(key(0xff08));assert(out.value.result.empty()&&out.value.composition.size()==253);
        assert(send(key(0xff08,true)).value.result.empty());
        // Equal text in separate selection operations is intentional input.
        out=send(mouse?select:key('1'));assert(out.value.result=="甲词"&&out.value.composition.size()==251);
        if(!mouse)assert(send(key('1',true)).value.result.empty());
        select.sym=1;
        out=send(mouse?select:key('2'));assert(out.value.result=="甲词"&&out.value.composition.empty());
        for(auto h:{key('2',true),key('z'),key('z',true)})assert(send(h).value.result.empty());
        assert(send(packet(Command::Clear)).value.result.empty());
        backend.reset();editing.reset();
        assert(send(key('a')).value.result.empty());
        assert(send(key('a',true)).value.result.empty());
    }
    // Query failure must not be treated as permission to replay old output.
    wchar_t output=L'x';std::string converted="old";
    auto assign=+[](void* object,const char* text)->void*{*static_cast<std::string*>(object)=text;return object;};
    for(auto get:{+[](void*,wchar_t**)->bool{return false;},+[](void*,wchar_t** out)->bool{*out=nullptr;return true;}}){
        ResultBuffers buffers(&output,get,&converted,assign);
        bool failed=false;try{buffers.clear();}catch(const std::runtime_error&){failed=true;}
        assert(failed&&output==L'x'&&converted=="old");
    }
}
