#include "scheduling.hpp"
#include <cassert>
#include <fcitx-utils/event.h>
using namespace sogo::direct;
int main(){
    PendingQueue queue;queue.focus(1);
    const std::uint64_t time=50000000;
    assert(nextWake(time,queue,true,true,time,0)==time+MaintenanceUs);
    auto key=packet(Command::Key);
    assert(queue.push(key,time));
    assert(nextWake(time,queue,true,true,time,0)==time+DeadlineUs);
    // Receiving another key must not postpone the oldest key's watchdog.
    auto second=packet(Command::Key);assert(queue.push(second,time+10000));
    assert(nextWake(time+10000,queue,true,true,time,0)==time+DeadlineUs);
    assert(nextWake(time+DeadlineUs-2000,queue,true,true,time,0)==time+DeadlineUs);
    assert(nextWake(time+DeadlineUs,queue,true,true,time,0)==time+DeadlineUs+1000);
    Pending done;assert(queue.take(key,done)==PendingQueue::Match::Complete);
    assert(nextWake(time+20000,queue,true,true,time,0)==time+10000+DeadlineUs);
    assert(queue.take(second,done)==PendingQueue::Match::Complete);
    assert(nextWake(time+30000,queue,true,true,time,0)==time+30000+MaintenanceUs);
    for(std::uint64_t offset:{30000,500000,999000})
        assert(nextWake(time+offset,queue,true,true,time,0,time+MaintenanceUs)==time+MaintenanceUs);
    assert(nextWake(time,queue,false,false,0,time+50000)==time+50000);
    assert(nextWake(time,queue,false,false,0,time+30000000)==time+MaintenanceUs);
    assert(nextWake(time,queue,false,false,0,time-1)==time+1000);
    assert(nextWake(time,queue,false,false,0,0)==time+MaintenanceUs);
    assert(nextWake(time,queue,true,false,time-StartupUs+20000,0)==time+20000);
    // The idle maintenance schedule has one wake per second and no key delay.
    auto current=time;unsigned wakes=0;
    while(current<time+60000000){current=nextWake(current,queue,true,true,time,0);++wakes;}
    assert(wakes==60);
    key=packet(Command::Key);assert(queue.push(key,current+500));
    assert(nextWake(current+500,queue,true,true,time,0)==current+500+DeadlineUs);
    // Exercise the same one-shot rearming on Fcitx's event loop without an
    // Instance, frontend, X11 or D-Bus connection.
    fcitx::EventLoop loop;PendingQueue live;live.focus(2);
    const auto begin=fcitx::now(CLOCK_MONOTONIC);
    auto request=packet(Command::Key);assert(live.push(request,begin));
    unsigned fired=0;
    auto watchdog=loop.addTimeEvent(CLOCK_MONOTONIC,begin+MaintenanceUs,1000,
        [&](fcitx::EventSourceTime*,std::uint64_t){
            ++fired;assert(live.expired(fcitx::now(CLOCK_MONOTONIC)));loop.exit();return true;
        });
    watchdog->setTime(nextWake(begin,live,true,true,begin,0));watchdog->setOneShot();
    bool timedOut=false;
    auto safety=loop.addTimeEvent(CLOCK_MONOTONIC,begin+MaintenanceUs,1000,
        [&](fcitx::EventSourceTime*,std::uint64_t){timedOut=true;loop.exit();return true;});
    loop.exec();assert(fired==1&&!timedOut);
}
