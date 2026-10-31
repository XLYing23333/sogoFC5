#pragma once
#include "shell_view.hpp"
#include <array>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <deque>
#include <stdexcept>
#include <string>
#include <sys/socket.h>

namespace sogo::direct {
constexpr std::uint32_t Magic=0x324f4753, Version=2;
constexpr std::uint32_t ReplyHandled=1,ReplyPageNext=2,ReplyPagePrevious=4,ReplyPageReset=8;
constexpr std::size_t MaxPayload=60000, MaxPending=128;
constexpr std::uint64_t DeadlineUs=180000;
enum class Command : std::uint32_t {Ready=1,Focus,Blur,Key,Select,Previous,Next,Clear,Stop,Reply,Fault};
struct Header {
    std::uint32_t magic=Magic,version=Version,type=0,size=0;
    std::uint64_t generation=0,context=0,sequence=0;
    std::uint32_t sym=0,state=0,code=0,release=0,time=0,handled=0;
    std::uint64_t engineUs=0;
};
static_assert(sizeof(Header)==72);
struct Message {Header header;std::string body;};
inline Header packet(Command type,std::uint64_t generation=0,std::uint64_t context=0,std::uint64_t sequence=0){
    Header h;h.type=static_cast<std::uint32_t>(type);h.generation=generation;h.context=context;h.sequence=sequence;return h;
}
inline bool send(int fd,Header h,const std::string& body={}){
    if(body.size()>MaxPayload)return false;
    h.size=body.size();
    iovec vectors[2]={{&h,sizeof(h)},{const_cast<char*>(body.data()),body.size()}};
    msghdr msg{};msg.msg_iov=vectors;msg.msg_iovlen=body.empty()?1:2;
    return ::sendmsg(fd,&msg,MSG_DONTWAIT|MSG_NOSIGNAL)==static_cast<ssize_t>(sizeof(h)+body.size());
}
inline int receive(int fd,Message& out){
    // recvmsg initializes exactly n bytes. Header length and payload size are
    // checked before accessing them; unused receive capacity is never read.
    std::array<char,sizeof(Header)+MaxPayload> bytes;
    iovec vector{bytes.data(),bytes.size()};msghdr msg{};msg.msg_iov=&vector;msg.msg_iovlen=1;
    auto n=recvmsg(fd,&msg,MSG_DONTWAIT);
    if(n<0)return errno==EAGAIN||errno==EWOULDBLOCK||errno==EINTR?0:-1;
    if(n<static_cast<ssize_t>(sizeof(Header))||(msg.msg_flags&MSG_TRUNC))return -1;
    std::memcpy(&out.header,bytes.data(),sizeof(Header));
    const auto& h=out.header;
    if(h.magic!=Magic||h.version!=Version||h.size>MaxPayload||n!=static_cast<ssize_t>(sizeof(Header)+h.size))return -1;
    out.body.assign(bytes.data()+sizeof(Header),h.size);return 1;
}
inline void append32(std::string& data,std::uint32_t value){
    for(unsigned i=0;i<4;++i)data.push_back(static_cast<char>((value>>(i*8))&255));
}
inline void appendString(std::string& data,const std::string& value){
    if(value.size()>MaxPayload)throw std::length_error("oversize result string");
    append32(data,value.size());data+=value;
}
inline std::string encode(const fc4::ShellView::Snapshot& value){
    if(value.candidates.size()>256)throw std::length_error("oversize result list");
    std::string out;append32(out,static_cast<std::uint32_t>(value.cursor));append32(out,value.hasCursor);
    appendString(out,value.composition);appendString(out,value.result);append32(out,value.candidates.size());
    for(const auto& word:value.candidates)appendString(out,word);
    if(out.size()>MaxPayload)throw std::length_error("oversize engine result");
    return out;
}
inline fc4::ShellView::Snapshot decode(const std::string& data){
    std::size_t pos=0;
    const auto integer=[&](){
        if(data.size()-pos<4)throw std::invalid_argument("truncated result");
        std::uint32_t n=0;for(unsigned i=0;i<4;++i)n|=std::uint32_t(static_cast<unsigned char>(data[pos++]))<<(i*8);return n;
    };
    const auto text=[&](){auto n=integer();if(n>data.size()-pos)throw std::invalid_argument("truncated text");auto s=data.substr(pos,n);pos+=n;return s;};
    fc4::ShellView::Snapshot value;value.cursor=static_cast<std::int32_t>(integer());
    const auto cursor=integer();if(cursor>1)throw std::invalid_argument("invalid cursor flag");value.hasCursor=cursor;
    value.composition=text();value.result=text();const auto count=integer();
    if(count>256)throw std::invalid_argument("invalid result count");
    for(std::uint32_t i=0;i<count;++i)value.candidates.push_back(text());
    if(pos!=data.size())throw std::invalid_argument("trailing result data");
    return value;
}
struct Pending {Header key;std::uint64_t queued;};
class PendingQueue {
public:
    std::uint64_t generation=1,context=0,sequence=0;
    std::deque<Pending> keys;
    void focus(std::uint64_t id){++generation;context=id;keys.clear();}
    bool push(Header& h,std::uint64_t time){
        if(keys.size()>=MaxPending)return false;
        h.generation=generation;h.context=context;h.sequence=++sequence;keys.push_back({h,time});return true;
    }
    enum class Match {Stale,Invalid,Complete};
    Match take(const Header& reply,Pending& out){
        if(reply.generation!=generation||reply.context!=context)return Match::Stale;
        if(keys.empty()||reply.sequence!=keys.front().key.sequence)return Match::Invalid;
        out=keys.front();keys.pop_front();return Match::Complete;
    }
    bool expired(std::uint64_t time)const{return !keys.empty()&&time-keys.front().queued>=DeadlineUs;}
    bool mayCompose()const{
        for(const auto& request:keys){
            const auto command=static_cast<Command>(request.key.type);
            if(command==Command::Key){if(!request.key.release)return true;}
            else return true;
        }
        return false;
    }
};
}
