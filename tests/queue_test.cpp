#include "protocol.hpp"
#include <cassert>
using namespace sogo::direct;
int main(){
    PendingQueue queue;queue.focus(1);auto first=packet(Command::Key);first.sym='a';assert(queue.push(first,100));
    assert(!queue.expired(100+DeadlineUs-1));assert(queue.expired(100+DeadlineUs));
    auto wrong=first;++wrong.sequence;Pending received;
    assert(queue.take(wrong,received)==PendingQueue::Match::Invalid);
    auto stale=first;queue.focus(2);assert(queue.take(stale,received)==PendingQueue::Match::Stale);
    for(std::size_t i=0;i<MaxPending;++i){auto h=packet(Command::Key);h.sym='a'+i%26;h.release=i%2;assert(queue.push(h,200+i));}
    auto overflow=packet(Command::Key);assert(!queue.push(overflow,300));
    auto copy=queue.keys;
    for(const auto& request:copy){assert(queue.take(request.key,received)==PendingQueue::Match::Complete);assert(received.key.sequence==request.key.sequence);}
    assert(queue.keys.empty());
    auto release=packet(Command::Key);release.sym='a';release.release=1;
    assert(queue.push(release,300)&&!queue.mayCompose());
    auto down=packet(Command::Key);down.sym='a';assert(queue.push(down,301)&&queue.mayCompose());
    assert(queue.take(release,received)==PendingQueue::Match::Complete&&queue.mayCompose());
    assert(queue.take(down,received)==PendingQueue::Match::Complete&&!queue.mayCompose());
    for(int batch=0;batch<100;++batch){
        for(int i=0;i<100;++i){auto h=packet(Command::Key);h.sym='a'+i%26;h.release=i%2;assert(queue.push(h,i));}
        while(!queue.keys.empty()){auto h=queue.keys.front().key;assert(queue.take(h,received)==PendingQueue::Match::Complete);}
    }
    sogo::fc4::ShellView::Snapshot value;value.composition="ni'hao";value.result="你好";value.candidates={"你好","拟好"};value.hasCursor=true;value.cursor=3;
    auto data=encode(value);auto decoded=decode(data);
    assert(decoded.composition==value.composition&&decoded.result==value.result&&decoded.candidates==value.candidates&&decoded.cursor==3);
    for(std::size_t i=0;i<data.size();++i){bool rejected=false;try{decode(data.substr(0,i));}catch(const std::invalid_argument&){rejected=true;}assert(rejected);}
    bool rejected=false;try{decode(data+"x");}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
}
