#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace sogo::fc4 {

// This interface is specific to the amd64 4.2.1.145 image in baseline.json.
// Objects and borrowed result interfaces remain owned by the vendor library.
// A snapshot must be copied before the next engine operation.
class ShellView {
public:
    struct Snapshot {
        std::string composition;
        std::string result;
        std::vector<std::string> candidates;
        std::int32_t cursor = -1;
        bool hasCursor = false;
    };

    explicit ShellView(void* object) : object_(object) {
        if (!object_) throw std::invalid_argument("null shell object");
    }

    void setAuthEnvironment(void* environment) const { call<void>(object_, 2, environment); }
    bool initialize(const char* system, const char* user, const char* conf,
                    const char* resources, const char* workspace) const {
        return call<bool>(object_, 4, system, user, conf, resources, workspace);
    }
    bool activate(std::int32_t layout) const { return call<bool>(object_, 5, layout); }
    void* resultObject() const { return call<void*>(object_, 19); }

    bool insertChar(std::int32_t code) const {
        return call<bool>(object_, 6, code);
    }
    bool back() const { return call<bool>(object_, 7); }
    void clear() const { call<void>(object_, 8); }
    void select(std::int32_t index) const {
        if (index < 0) throw std::out_of_range("negative candidate index");
        call<void>(object_, 9, index);
    }
    bool pageDown() const { return call<bool>(object_, 13); }
    bool pageUp() const { return call<bool>(object_, 14); }
    bool setCursor(bool flag, std::int32_t position) const {
        return call<bool>(object_, 25, flag, position);
    }
    std::int32_t integerInfo(std::uint32_t parameter) const {
        return call<std::int32_t>(object_, 28, parameter);
    }

    Snapshot snapshot() const {
        Snapshot value;
        value.composition = text(call<void*>(object_, 17));
        value.result = text(call<void*>(object_, 19));
        void* list = call<void*>(object_, 16);
        if (list) {
            const auto count = call<std::int32_t>(list, 1);
            if (count < 0 || count > 256)
                throw std::length_error("invalid candidate count");
            value.candidates.reserve(static_cast<std::size_t>(count));
            for (std::int32_t index = 0; index < count; ++index)
                value.candidates.push_back(text(call<void*>(list, 0, static_cast<std::uint64_t>(index))));
        }
        value.hasCursor = call<bool>(object_, 26, &value.cursor);
        return value;
    }

private:
    template<class R, class... Args>
    static R call(void* object, std::size_t index, Args... args) {
        if (!object) throw std::invalid_argument("null ABI object");
        const void* table = nullptr;
        std::memcpy(&table, object, sizeof(table));
        if (!table) throw std::invalid_argument("null ABI vtable");
        using Function = R (*)(void*, Args...);
        Function function = nullptr;
        static_assert(sizeof(function) == sizeof(void*));
        const auto* slot = static_cast<const unsigned char*>(table) + index * sizeof(void*);
        std::memcpy(&function, slot, sizeof(function));
        if (!function) throw std::invalid_argument("null ABI function");
        return function(object, args...);
    }

    static std::string text(void* object) {
        if (!object) return {};
        const char* value = call<const char*>(object, 0);
        if (!value) return {};
        constexpr std::size_t limit = 65536;
        const auto size = ::strnlen(value, limit);
        if (size == limit) throw std::length_error("engine text exceeds limit");
        return {value, size};
    }

    void* object_;
};

} // namespace sogo::fc4
