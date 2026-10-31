#include "profile.hpp"
#include <cassert>
#include <iterator>
#include <sys/stat.h>
using namespace sogo::direct;
int main(){
    char name[]="/tmp/sogo-direct-profile-XXXXXX";auto* temp=mkdtemp(name);assert(temp);
    const std::filesystem::path root=temp;const auto profile=prepareProfile(root);
    struct stat mode{};assert(stat(profile.c_str(),&mode)==0&&(mode.st_mode&0777)==0700);
    std::ifstream input(profile/"conf/env.ini");const std::string text{std::istreambuf_iterator<char>(input),{}};
    assert(text.find("KeyCanSwitch=0\n")!=std::string::npos&&text.find("SymbolCompletion=0\n")!=std::string::npos);
    assert(text.find("PadNumAsSelect=0\n")!=std::string::npos);
    assert(std::filesystem::equivalent(profile/"conf/env.ini",profile/"dict/PCPYDict/env.ini"));
    assert(std::filesystem::equivalent(profile/"conf/env.ini",profile/"dict/conf/env.ini"));
    {std::ofstream saved(profile/"dict/learning.bin");saved<<"fixture-data";}
    {std::ofstream saved(profile/"conf/env.ini");saved<<"[Setting]\nLearnFreq=0\n";}
    prepareProfile(root);
    assert(std::filesystem::file_size(profile/"dict/learning.bin")==12);
    std::ifstream check(profile/"conf/env.ini");std::string saved{std::istreambuf_iterator<char>(check),{}};
    assert(saved=="[Setting]\nLearnFreq=0\n");
    int lock=acquireProfileLock(root);bool rejected=false;
    try{acquireProfileLock(root);}catch(const std::runtime_error&){rejected=true;}
    assert(rejected);close(lock);
    std::filesystem::remove_all(root);
}
