#pragma once
#include <algorithm>
#include <array>
#include <cstdint>

namespace sogo::direct {
class Latency {
public:
    static constexpr std::array<std::uint64_t,12> bounds{250,500,1000,2000,4000,8000,16000,32000,64000,128000,180000,1000000};
    void add(std::uint64_t us){
        ++count_;total_+=us;maximum_=std::max(maximum_,us);
        const auto index=std::lower_bound(bounds.begin(),bounds.end(),us)-bounds.begin();
        ++buckets_[index];
    }
    std::uint64_t count()const{return count_;}
    std::uint64_t mean()const{return count_?total_/count_:0;}
    std::uint64_t maximum()const{return maximum_;}
    // Histogram percentiles are upper bounds, not exact sample percentiles.
    std::uint64_t percentileUpper(unsigned percent)const{
        if(!count_)return 0;
        const auto rank=(count_*std::clamp(percent,1u,100u)+99)/100;
        std::uint64_t accumulated=0;
        for(std::size_t i=0;i<buckets_.size();++i){
            accumulated+=buckets_[i];
            if(accumulated>=rank)return std::min(maximum_,i<bounds.size()?bounds[i]:maximum_);
        }
        return maximum_;
    }
private:
    std::array<std::uint64_t,bounds.size()+1> buckets_{};
    std::uint64_t count_=0,total_=0,maximum_=0;
};
}
