#pragma once
#include "keys.hpp"
#include "protocol.hpp"
#include <array>
#include <optional>

namespace sogo::direct {
// Editing state belongs to one engine context and follows acknowledged engine
// snapshots. Paging must not depend on a desktop panel that can lag the queue.
class Editing {
public:
    struct Outcome {fc4::ShellView::Snapshot value;std::uint32_t flags=0;};
    void reset(){composition_.clear();candidates_=false;paging_.fill(0);keypad_.clear();}
    template<class Backend> Outcome process(Backend& backend,const Header& h){
        const auto command=static_cast<Command>(h.type);
        bool handled=false;int moved=0;
        std::optional<fc4::ShellView::Snapshot> captured;
        const auto flip=[&](int direction){
            backend.begin();
            if(!composition_.empty()&&candidates_){
                const auto before=backend.snapshot();
                if(direction>0)backend.next();else backend.previous();
                captured=backend.snapshot();
                // The PC Shell returns true even at a page boundary. Confirm
                // a visible candidate transition rather than trust that flag.
                if(before.candidates!=captured->candidates)moved=direction;
            }
        };
        if(command==Command::Key){
            auto vk=virtualKey(h.sym);
            if(h.code){
                if(h.release){
                    const auto pressed=keypad_.find(h.code);
                    if(pressed!=keypad_.end()){vk=pressed->second;keypad_.erase(pressed);}
                }else if(keypadVirtualKey(vk))keypad_[h.code]=vk;
            }
            const auto direction=pageDirection(h.sym,h.state);
            if(vk&&paging_[vk]){
                handled=true;
                if(h.release){paging_[vk]=0;backend.begin();}
                else flip(paging_[vk]);
            }else if(!h.release&&direction&&!composition_.empty()&&candidates_){
                paging_[vk]=direction;handled=true;flip(direction);
            }else if(vk==8){
                if(!h.release&&!composition_.empty()){
                    // Each compositor repeat is a complete internal stroke.
                    // This accepts both press-only and press/release repeat
                    // frontends and leaves no vendor Backspace held down when
                    // the remaining repeats return to the application.
                    backend.key(vk,h.state,false);
                    auto pressed=backend.snapshot();
                    backend.key(vk,h.state,true);
                    captured=backend.snapshot();
                    captured->result=pressed.result+captured->result;
                    handled=true;
                }else backend.begin();
            }else if(vk)handled=backend.key(vk,h.state,h.release);
            else backend.begin();
        }else if(command==Command::Previous||command==Command::Next){
            handled=true;flip(command==Command::Next?1:-1);
        }else if(command==Command::Select){
            backend.select(static_cast<std::int32_t>(h.sym));handled=true;
        }else if(command==Command::Clear){
            backend.reset();paging_.fill(0);keypad_.clear();handled=true;
        }else throw std::runtime_error("unsupported editing command");
        Outcome out;out.value=captured?std::move(*captured):backend.snapshot();
        if(handled||!out.value.result.empty()||!out.value.composition.empty())out.flags|=ReplyHandled;
        if(!out.value.result.empty()||composition_!=out.value.composition)out.flags|=ReplyPageReset;
        else if(moved>0)out.flags|=ReplyPageNext;
        else if(moved<0)out.flags|=ReplyPagePrevious;
        composition_=out.value.composition;candidates_=!out.value.candidates.empty();
        return out;
    }
private:
    std::string composition_;bool candidates_=false;std::array<int,256> paging_{};
    std::unordered_map<std::uint32_t,std::uint8_t> keypad_;
};
}
