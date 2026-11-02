#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace sogo::direct {
// The pinned native wrapper initializes its status output to zero. A returned
// call is not independent proof that every vendor dictionary reached storage.
class NativeLearning {
public:
    using Factory = void* (*)();
    using Save = void (*)(void*, const char*, int*);
    NativeLearning(Factory factory, Save regular, Save sync)
        : factory_(factory), regular_(regular), sync_(sync) {}
    void save(bool sync) const {
        void* owner = factory_();
        void* core = nullptr;
        if (owner) std::memcpy(&core, owner, sizeof(core));
        if (!core) throw std::runtime_error("native learning core unavailable");
        int status = -1;
        (sync ? sync_ : regular_)(owner, "Sogo-FC5", &status);
        if (status) throw std::runtime_error("native learning save failed");
    }
private:
    Factory factory_;
    Save regular_, sync_;
};

class LearningCheckpoint {
public:
    static constexpr std::uint64_t IdleUs = 2000000, IntervalUs = 30000000,
                                   RetryUs = 5000000;
    void input(std::uint64_t time, bool committed) {
        lastInput_ = time;
        if (committed && !dirty_) { dirty_ = true; dirtySince_ = time; }
    }
    bool due(std::uint64_t time) const {
        return dirty_ && time >= retryAt_ &&
            (time - lastInput_ >= IdleUs || time - dirtySince_ >= IntervalUs);
    }
    template<class Backend> bool save(Backend& backend, bool sync, std::uint64_t time) {
        if (!sync && !dirty_) return true;
        if (!sync && time < retryAt_) return false;
        ++attempts;
        try {
            backend.saveLearning(sync);
            ++returned; dirty_ = false; retryAt_ = 0;
            if (sync) ++syncReturned;
            return true;
        } catch (...) {
            ++failures;
            if (!dirty_) { dirty_ = true; dirtySince_ = time; }
            retryAt_ = time + RetryUs;
            return false;
        }
    }
    bool dirty() const { return dirty_; }
    std::uint64_t attempts = 0, returned = 0, syncReturned = 0, failures = 0;
private:
    bool dirty_ = false;
    std::uint64_t lastInput_ = 0, dirtySince_ = 0, retryAt_ = 0;
};
}
