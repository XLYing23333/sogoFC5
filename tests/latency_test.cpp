#include "latency.hpp"
#include <cassert>
using namespace sogo::direct;
int main(){
    Latency value;
    assert(value.mean()==0&&value.maximum()==0&&value.percentileUpper(95)==0);
    for(int i=0;i<95;++i)value.add(200);
    for(int i=0;i<4;++i)value.add(2501);
    value.add(2000000);
    assert(value.count()==100&&value.maximum()==2000000);
    assert(value.mean()==20290);
    assert(value.percentileUpper(95)==250&&value.percentileUpper(99)==4000);
    assert(value.percentileUpper(100)==2000000);
    Latency single;single.add(321);
    assert(single.percentileUpper(95)==321);
    for(unsigned percentile:{1u,50u,95u,99u,100u}){
        auto lower=value.percentileUpper(percentile);
        assert(lower<=value.maximum());
    }
}
