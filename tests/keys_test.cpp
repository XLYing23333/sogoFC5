#include "keys.hpp"
#include <cassert>
using namespace sogo::direct;
int main(){
    for(unsigned i=0;i<10;++i){
        assert(keypadDigit(0xffb0+i)==static_cast<int>(i));
        assert(virtualKey(0xffb0+i)==0x60+i&&!original(0xffb0+i,16));
        for(unsigned state:{0u,1u,16u,17u,RepeatState|1u}){
            const auto normalized=engineState(0x60+i,state);std::array<unsigned char,256> cache{};
            modifiers(cache.data(),normalized);assert(cache[0x10]==0&&cache[0x90]==1);
            assert(normalized==((state&~1u)|16u));
        }
    }
    assert(keypadDigit('1')==-1&&keypadDigit(0xff9c)==-1);
    assert(virtualKey(0xff9c)==0&&engineState('1',17)==17);
    for(unsigned c='a';c<='z';++c){
        assert(virtualKey(c)==c-'a'+'A');
        assert(static_cast<std::uint32_t>(shellKey(virtualKey(c),false))==(0x80000000u|virtualKey(c)));
        assert(shellKey(virtualKey(c),true)==virtualKey(c));
        assert(!original(c,0)&&!original(c,1)&&!original(c,2));
        for(unsigned state:{4u,5u,8u,9u,64u,128u,256u,512u})assert(original(c,state));
    }
    assert(virtualKey('(')=='9'&&virtualKey(')')=='0');
    assert(virtualKey('[')==0xdb&&virtualKey('{')==0xdb);
    assert(virtualKey(';')==0xba&&virtualKey(':')==0xba);
    assert(virtualKey(0xff08)==8&&virtualKey(0xff0d)==13);
    assert(original(' ',1));
    assert(pageDirection('=',0)==1&&pageDirection('-',0)==-1);
    assert(pageDirection(0xff56,RepeatState)==1&&pageDirection(0xff55,2|16)==-1);
    for(unsigned state:{1u,4u,8u,32u,64u,128u,1u<<26,1u<<27,1u<<28}){
        assert(!pageDirection('=',state)&&!pageDirection('-',state));
    }
    assert(!pageDirection('+',1)&&!pageDirection('_',1));
    assert(applicationState(RepeatState|5)==5&&applicationState(0)==0);
    KeyRoutes routes;
    assert(!routes.native('C',5,54,false));
    assert(!routes.native('c',0,54,true));
    assert(!routes.native('V',5,55,false));
    assert(!routes.native('v',0,55,true));
    assert(!routes.native(0xffe1,0,50,false));
    assert(routes.native('A',1,38,false));
    assert(routes.native('a',0,38,true));
    assert(!routes.native(0xffe1,0,50,true));
    assert(!routes.native(0xff08,0,22,false,false));
    assert(!routes.native(0xff08,0,22,true,true));
    assert(routes.native(0xff08,0,22,false,true));
    for(int i=0;i<100;++i)assert(routes.native(0xff08,RepeatState,22,false,true));
    assert(!routes.native(0xff08,RepeatState,22,false,false));
    assert(!routes.native(0xff08,0,22,true,true));
    std::array<unsigned char,256> state{};
    state['A']=0x80;modifiers(state.data(),1|2|16);
    assert(state[0x10]==0x80&&state[0xa0]==0x80&&state[0x14]==1&&state[0x90]==1);
    assert(state['A']==0x80&&state[0x11]==0&&state[0x12]==0);
    modifiers(state.data(),0);
    assert(state[0x10]==0&&state[0x14]==0&&state[0x90]==0&&state['A']==0x80);
    for(int i=0;i<10000;++i){assert(routes.native('a',0,38,false));assert(routes.native('a',0,38,true));}
}
