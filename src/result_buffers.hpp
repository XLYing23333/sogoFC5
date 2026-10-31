#pragma once
#include <stdexcept>

namespace sogo::direct {
// The Shell's UTF-8 result is a view converted from a separate, writable
// core result. Both are per-operation output, independent of the preedit.
class ResultBuffers {
public:
    static_assert(sizeof(wchar_t)==4,"pinned amd64 core requires 32-bit wchar_t");
    using GetCore = bool (*)(void*, wchar_t**);
    using Assign = void* (*)(void*, const char*);
    ResultBuffers(void* core, GetCore get, void* converted, Assign assign)
        : core_(core), get_(get), converted_(converted), assign_(assign) {
        if(!core_||!get_||!converted_||!assign_)
            throw std::runtime_error("missing vendor result interface");
    }
    void clear() const {
        wchar_t* output=nullptr;
        if(!get_(core_,&output)||!output)
            throw std::runtime_error("missing vendor core result buffer");
        // Native transient-output reset also terminates this borrowed buffer
        // at its first wchar. Do not reset input, candidates or learning state.
        *output=L'\0';
        assign_(converted_,"");
    }
private:
    void* core_;GetCore get_;void* converted_;Assign assign_;
};
}
