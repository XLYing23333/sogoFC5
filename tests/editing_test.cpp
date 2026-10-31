#include "editing.hpp"
#include <cassert>
#include <vector>
using namespace sogo::direct;
class Backend {
public:
    void begin(){value.result.clear();}
    bool key(unsigned char vk,std::uint32_t,bool release){
        begin();strokes.push_back({vk,release});
        if(release){held[vk]=false;return false;}
        if(held[vk])return false;
        held[vk]=true;
        if(vk==8){
            if(value.composition.empty())return false;
            value.composition.pop_back();page=0;refresh();
            // Empty preedit does not imply that this delete was unhandled.
            return !value.composition.empty();
        }
        if(vk>='A'&&vk<='Z')value.composition+=static_cast<char>(vk-'A'+'a');
        else value.result=vk==0xbb?"=":vk==0xbd?"-":"?";
        refresh();return true;
    }
    bool next(){begin();++nextCalls;if(page<2)++page;refresh();return true;}
    bool previous(){begin();++previousCalls;if(page)--page;refresh();return true;}
    void select(std::int32_t){begin();value.result="你好";value.composition.clear();page=0;refresh();}
    void reset(){value={};held.fill(false);page=0;}
    auto snapshot()const{return value;}
    int page=0,nextCalls=0,previousCalls=0;
    std::vector<std::pair<unsigned char,bool>> strokes;
    std::array<bool,256> held{};
private:
    void refresh(){value.candidates=value.composition.empty()?std::vector<std::string>{}:std::vector<std::string>{"page-"+std::to_string(page),"拟好","你号","泥好","倪好"};}
    sogo::fc4::ShellView::Snapshot value;
};
Header key(unsigned sym,bool release=false,std::uint32_t state=0){auto h=packet(Command::Key);h.sym=sym;h.release=release;h.state=state;return h;}
int main(){
    Backend backend;Editing editing;
    auto out=editing.process(backend,key('='));assert(out.value.result=="="&&backend.nextCalls==0);
    editing.process(backend,key('=',true));
    out=editing.process(backend,key('-'));assert(out.value.result=="-"&&backend.previousCalls==0);
    editing.process(backend,key('-',true));
    editing.process(backend,key('a'));editing.process(backend,key('a',true));
    const auto count=backend.strokes.size();
    out=editing.process(backend,key('='));assert(out.flags==(ReplyHandled|ReplyPageNext)&&backend.page==1);
    out=editing.process(backend,key('=',true));assert(out.flags==ReplyHandled&&backend.page==1);
    out=editing.process(backend,key('=',false,RepeatState));assert(out.flags==(ReplyHandled|ReplyPageNext)&&backend.page==2);
    out=editing.process(backend,key('=',false,RepeatState));assert(out.flags==ReplyHandled&&backend.page==2);
    editing.process(backend,key('=',true));
    out=editing.process(backend,key('-'));assert(out.flags==(ReplyHandled|ReplyPagePrevious)&&backend.page==1);
    // Holding '-' preserves its direction even if Shift changes mid-stroke.
    out=editing.process(backend,key('_',false,1|RepeatState));assert(out.flags==(ReplyHandled|ReplyPagePrevious)&&backend.page==0);
    out=editing.process(backend,key('-',false,RepeatState));assert(out.flags==ReplyHandled&&backend.page==0);
    editing.process(backend,key('-',true));
    assert(backend.strokes.size()==count);
    out=editing.process(backend,packet(Command::Next));assert(out.flags==(ReplyHandled|ReplyPageNext));
    out=editing.process(backend,packet(Command::Previous));assert(out.flags==(ReplyHandled|ReplyPagePrevious));
    out=editing.process(backend,packet(Command::Previous));assert(out.flags==ReplyHandled);
    auto before=backend.nextCalls;
    out=editing.process(backend,key('+',false,1));assert(out.value.result=="="&&backend.nextCalls==before);
    editing.process(backend,key('+',true,1));
    // A new physical punctuation press must not inherit an old page-key hold.
    editing.process(backend,key('='));backend.reset();editing.reset();
    out=editing.process(backend,key('='));assert(out.value.result=="=");
    editing.process(backend,key('=',true));
    for(bool releases:{false,true}){
        backend.reset();editing.reset();
        for(int i=0;i<80;++i){editing.process(backend,key('a'));editing.process(backend,key('a',true));}
        unsigned consumed=0,passed=0;const auto first=backend.strokes.size();
        for(int i=0;i<120;++i){
            out=editing.process(backend,key(0xff08,false,i?RepeatState:0));
            assert(out.value.result.empty());
            if(out.flags&ReplyHandled)++consumed;else ++passed;
            assert(!backend.held[8]);
            if(releases)editing.process(backend,key(0xff08,true));
        }
        out=editing.process(backend,key(0xff08,true));
        assert(consumed==80&&passed==40&&out.value.composition.empty()&&out.value.candidates.empty());
        assert(backend.strokes.size()==first+160);
        for(auto i=first;i<backend.strokes.size();i+=2){
            assert(backend.strokes[i]==std::make_pair(static_cast<unsigned char>(8),false));
            assert(backend.strokes[i+1]==std::make_pair(static_cast<unsigned char>(8),true));
        }
    }
    // Deleting preedit must never replay the preceding committed text.
    backend.reset();editing.reset();editing.process(backend,key('a'));editing.process(backend,key('a',true));
    out=editing.process(backend,packet(Command::Select));assert(out.value.result=="你好");
    out=editing.process(backend,key(0xff08));assert(!(out.flags&ReplyHandled)&&out.value.result.empty());
    out=editing.process(backend,key(0xff08,true));assert(out.value.result.empty());
}
