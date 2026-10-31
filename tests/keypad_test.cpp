#include "editing.hpp"
#include <cassert>

using namespace sogo::direct;
class Backend {
public:
    void begin(){value.result.clear();}
    bool key(unsigned char vk,std::uint32_t,bool release){
        begin();last=vk;released=release;
        if(release)return false;
        if(keypadVirtualKey(vk))value.composition+=char('0'+vk-0x60);
        else if(vk>='A'&&vk<='Z')value.composition+=char('a'+vk-'A');
        else if(vk==8){if(!value.composition.empty())value.composition.pop_back();}
        else if(vk=='1'){select(0);return true;}
        refresh();return true;
    }
    void select(std::int32_t){begin();value.result=value.composition;value.composition.clear();refresh();}
    bool previous(){begin();return false;}
    bool next(){begin();return false;}
    void reset(){value={};last=0;released=false;}
    auto snapshot()const{return value;}
    unsigned char last=0;bool released=false;
private:
    void refresh(){value.candidates=value.composition.empty()?std::vector<std::string>{}:std::vector<std::string>{value.composition};}
    sogo::fc4::ShellView::Snapshot value;
};
Header key(unsigned sym,bool release=false,unsigned code=0,std::uint32_t state=0){auto h=packet(Command::Key);h.sym=sym;h.release=release;h.code=code;h.state=state;return h;}
int main(){
    Backend backend;Editing editing;KeyRoutes routes;
    auto send=[&](Header h){
        assert(routes.native(h.sym,h.state,h.code,h.release));
        return editing.process(backend,h);
    };
    send(key('n',false,57));send(key('n',true,57));
    for(unsigned i=0;i<10;++i){
        auto out=send(key(0xffb0+i,false,80+i,16));
        assert(out.value.result.empty()&&(out.flags&ReplyHandled));
        assert(backend.last==0x60+i&&backend.last!=virtualKey('0'+i));
        out=send(key(0xffb0+i,true,80+i,16));assert(out.value.result.empty());
    }
    auto out=send(key('h',false,43));assert(out.value.composition=="n0123456789h");
    send(key('h',true,43));
    out=send(key(0xff08,false,22));assert(out.value.composition=="n0123456789");
    send(key(0xff08,true,22));
    out=send(key('1',false,10));assert(out.value.result=="n0123456789"&&out.value.composition.empty());
    assert(send(key('1',true,10)).value.result.empty());
    // XKB can change a keypad keysym between press and release when NumLock
    // changes. The engine must still receive the release of the pressed VK.
    send(key('n',false,57));send(key('n',true,57));
    send(key(0xffb1,false,87,16));
    out=send(key(0xff9c,true,87,0));
    assert(out.value.result.empty()&&out.value.composition=="n1"&&backend.last==0x61&&backend.released);
    backend.reset();editing.reset();routes.reset();
    auto before=backend.last;
    editing.process(backend,key(0xff9c,true,87));assert(backend.last==before);
    assert(!routes.native(0xffb1,16,87,false,false));
    assert(!routes.native(0xffb1,16,87,true,true));
    for(unsigned state:{4u,5u,8u,64u}){
        assert(!routes.native(0xffb1,state,87,false));
        assert(!routes.native(0xffb1,0,87,true));
    }
}
