#pragma once
#include "protocol.hpp"
#include <algorithm>

namespace sogo::direct {
constexpr std::uint64_t MaintenanceUs=1000000,StartupUs=10000000;
inline std::uint64_t nextWake(std::uint64_t time,const PendingQueue& pending,
                             bool child,bool ready,std::uint64_t started,std::uint64_t retry,
                             std::uint64_t maintenance=0){
    auto next=maintenance?maintenance:time+MaintenanceUs;
    if(!pending.keys.empty())next=std::min(next,pending.keys.front().queued+DeadlineUs);
    if(child&&!ready)next=std::min(next,started+StartupUs);
    if(!child&&retry)next=std::min(next,retry);
    // An overdue deadline is handled on the next event-loop turn. Retrying a
    // failed spawn must not create a zero-delay busy loop.
    return std::max(next,time+1000);
}
}
