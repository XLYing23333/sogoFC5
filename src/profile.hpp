#pragma once
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

namespace sogo::direct {
inline int acquireProfileLock(const std::filesystem::path& root){
    int fd=open((root/"engine.lock").c_str(),O_RDWR|O_CREAT|O_NOFOLLOW|O_CLOEXEC,0600);
    struct stat info{};
    if(fd<0||fstat(fd,&info)||!S_ISREG(info.st_mode)||info.st_uid!=getuid()||flock(fd,LOCK_EX|LOCK_NB)){
        if(fd>=0)close(fd);
        throw std::runtime_error("private profile is already in use or unavailable");
    }
    return fd;
}
inline std::filesystem::path dataRoot(){
    const auto data=std::getenv("XDG_DATA_HOME"),home=std::getenv("HOME");
    if(data&&*data&&std::filesystem::path(data).is_absolute())return std::filesystem::path(data)/"sogo-fc5-direct";
    if(!home||!*home||!std::filesystem::path(home).is_absolute())throw std::runtime_error("absolute HOME is required");
    return std::filesystem::path(home)/".local/share/sogo-fc5-direct";
}
inline void privateDirectory(const std::filesystem::path& path){
    std::filesystem::create_directories(path);
    struct stat status{};
    if(lstat(path.c_str(),&status)||!S_ISDIR(status.st_mode)||status.st_uid!=getuid())
        throw std::runtime_error("invalid private directory");
    if(chmod(path.c_str(),0700))throw std::runtime_error("cannot secure private directory");
}
inline std::filesystem::path prepareProfile(const std::filesystem::path& root){
    privateDirectory(root);const auto home=root/"home";privateDirectory(home);
    privateDirectory(root/"cache");privateDirectory(root/"data");
    privateDirectory(home/".config");auto profile=home/".config/sogoupinyin";privateDirectory(profile);
    privateDirectory(profile/"dict");privateDirectory(profile/"conf");privateDirectory(profile/"log");
    const auto config=profile/"conf/env.ini";
    int fd=open(config.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
    if(fd>=0){
        constexpr char defaults[]="[Setting]\nDefaultNative=1\nDefaultEnglishInput=0\nDefaultPinyin=1\nDefaultShape=0\nDefaultTradition=0\nKeyCanSwitch=0\nKeyEnCnSwitch=2\nShortCutEnglishInput=0\nShortCutShape=0\nShortcutPunct=0\nShortCutFanJian=0\nActiveTempPY=0\nShortcutWBInputStyleNew=0\nQuickInputOnOff=0\nSymbolCompletion=0\nEngFill=0\nMailUrlFill=0\nKeyPageUpDown=0\nPadNumAsSelect=0\nPageSize=5\nLearnFreq=1\nLearnNewWord=1\nEnableCloudRequest=0\nQuanPin=1\nShuangPin=0\n\n";
        auto count=write(fd,defaults,sizeof(defaults)-1);close(fd);
        if(count!=static_cast<ssize_t>(sizeof(defaults)-1))throw std::runtime_error("cannot initialize profile");
    }else if(errno!=EEXIST)throw std::runtime_error("cannot create private profile");
    for(const auto& dict:{"PCPYDict","PCWBDict"}){
        privateDirectory(profile/"dict"/dict);
        const auto path=profile/"dict"/dict/"env.ini";
        if(!std::filesystem::exists(path)&&!std::filesystem::is_symlink(path))
            std::filesystem::create_symlink("../../conf/env.ini",path);
    }
    if(!std::filesystem::exists(profile/"dict/conf"))std::filesystem::create_directory_symlink("../conf",profile/"dict/conf");
    setenv("HOME",home.c_str(),1);setenv("XDG_CONFIG_HOME",(home/".config").c_str(),1);
    setenv("XDG_CACHE_HOME",(root/"cache").c_str(),1);setenv("XDG_DATA_HOME",(root/"data").c_str(),1);
    return profile;
}
}
