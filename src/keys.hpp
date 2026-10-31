#pragma once
#include <array>
#include <cstdint>
#include <unordered_map>

namespace sogo::direct {
inline int keypadDigit(std::uint32_t sym){return sym>=0xffb0&&sym<=0xffb9?static_cast<int>(sym-0xffb0):-1;}
inline bool keypadVirtualKey(std::uint8_t vk){return vk>=0x60&&vk<=0x69;}
inline std::uint32_t engineState(std::uint8_t vk,std::uint32_t state){
    // A resolved KP digit is a character even when XKB produced it through
    // Shift with NumLock off. Match that character in the native PC cache.
    return keypadVirtualKey(vk)?(state&~1u)|16u:state;
}
// X11 keysyms map to the PC shell's virtual-key interface, rather than ASCII.
inline std::uint8_t virtualKey(std::uint32_t sym) {
    if(const auto digit=keypadDigit(sym);digit>=0)return 0x60+digit;
    if (sym >= 'a' && sym <= 'z') return static_cast<std::uint8_t>(sym-'a'+'A');
    if ((sym >= 'A' && sym <= 'Z') || (sym >= '0' && sym <= '9')) return sym;
    switch (sym) {
    case 0xff08: return 8; case 0xff09: return 9; case 0xff0d: return 13;
    case 0xff1b: return 27; case ' ': return 32;
    case 0xff50: return 36; case 0xff51: return 37; case 0xff52: return 38;
    case 0xff53: return 39; case 0xff54: return 40; case 0xff55: return 33;
    case 0xff56: return 34; case 0xff57: return 35; case 0xffff: return 46;
    case ';': case ':': return 0xba; case '=': case '+': return 0xbb;
    case ',': case '<': return 0xbc; case '-': case '_': return 0xbd;
    case '.': case '>': return 0xbe; case '/': case '?': return 0xbf;
    case '`': case '~': return 0xc0; case '[': case '{': return 0xdb;
    case '\\': case '|': return 0xdc; case ']': case '}': return 0xdd;
    case '\'': case '"': return 0xde;
    case '!': return '1'; case '@': return '2'; case '#': return '3';
    case '$': return '4'; case '%': return '5'; case '^': return '6';
    case '&': return '7'; case '*': return '8'; case '(': return '9'; case ')': return '0';
    default: return 0;
    }
}
inline bool modifier(std::uint32_t sym) {
    return (sym>=0xffe1 && sym<=0xffee) || sym==0xfe03;
}
constexpr std::uint32_t RepeatState=1u<<31;
inline std::uint32_t applicationState(std::uint32_t state){return state&~RepeatState;}
inline int pageDirection(std::uint32_t sym,std::uint32_t state){
    // CapsLock, NumLock and repeat are not paging modifiers. Shift must keep
    // '+' and '_' available rather than turn their physical keys into paging.
    constexpr std::uint32_t modifiers=1u|4u|8u|32u|64u|128u|(1u<<26)|(1u<<27)|(1u<<28);
    if(state&modifiers)return 0;
    if(sym=='='||sym==0xff56)return 1;
    if(sym=='-'||sym==0xff55)return -1;
    return 0;
}
inline bool original(std::uint32_t sym, std::uint32_t state) {
    return modifier(sym) || (state & (4u|8u|64u|128u|256u|512u)) ||
           ((state&1u)&&sym==' ') ||
           (sym>=0xffbe && sym<=0xffe0) || !virtualKey(sym);
}
inline std::int32_t shellKey(std::uint8_t vk, bool release) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(vk) | (release ? 0u : 0x80000000u));
}
inline void modifiers(unsigned char* keys, std::uint32_t state) {
    keys[0x10]=keys[0xa0]=(state&1)?0x80:0; keys[0xa1]=0;
    keys[0x11]=keys[0xa2]=(state&4)?0x80:0; keys[0xa3]=0;
    keys[0x12]=keys[0xa4]=(state&8)?0x80:0; keys[0xa5]=0;
    keys[0x14]=(state&2)?1:0; keys[0x90]=(state&16)?1:0;
}

// Routing is paired by physical key, so releasing Ctrl before C does not send
// the release of an application shortcut into the engine.
class KeyRoutes {
public:
    bool native(std::uint32_t sym,std::uint32_t state,std::uint32_t code,bool release,bool allow=true) {
        auto id=code ? (std::uint64_t(1)<<32)|code :
            (sym>='A' && sym<='Z'?sym-'A'+'a':sym);
        auto found=routes_.find(id);
        if(release) {
            if(found==routes_.end()) return false;
            const bool result=found->second;routes_.erase(found);return result;
        }
        const bool route=allow&&!original(sym,state);
        routes_[id]=route;
        return route;
    }
    void reset(){routes_.clear();}
private:
    std::unordered_map<std::uint64_t,bool> routes_;
};
}
