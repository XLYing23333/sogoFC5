#include "learning.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <unistd.h>
using namespace sogo::direct;
namespace {
struct Owner { void* core; } owner{&owner};
bool failNative = false; int regularCalls = 0, syncCalls = 0;
void* factory() { return &owner; }
void regular(void* object, const char* reason, int* status) {
    assert(object == &owner && std::string(reason) == "Sogo-FC5");
    ++regularCalls; *status = failNative ? 1 : 0;
}
void sync(void* object, const char* reason, int* status) {
    assert(object == &owner && std::string(reason) == "Sogo-FC5");
    ++syncCalls; *status = 0;
}
struct Store {
    std::filesystem::path path; std::string value;
    unsigned saves = 0; bool fail = false;
    void saveLearning(bool) {
        ++saves;
        if (fail) throw std::runtime_error("fixture failure");
        std::ofstream file(path); file << value;
        if (!file) throw std::runtime_error("fixture storage failure");
    }
};
}
int main() {
    NativeLearning native(factory, regular, sync);
    native.save(false); native.save(true);
    assert(regularCalls == 1 && syncCalls == 1);
    owner.core = nullptr; bool rejected = false;
    try { native.save(false); } catch (const std::runtime_error&) { rejected = true; }
    assert(rejected && regularCalls == 1);
    owner.core = &owner; failNative = true; rejected = false;
    try { native.save(false); } catch (const std::runtime_error&) { rejected = true; }
    assert(rejected);

    char name[] = "/tmp/sogo-learning-XXXXXX";
    auto* directory = mkdtemp(name); assert(directory);
    const auto path = std::filesystem::path(directory) / "dictionary";
    Store store{path, "fixture-phrase"}; LearningCheckpoint state;
    const std::uint64_t start = 100000000;
    assert(!state.due(start));
    assert(state.save(store, false, start) && store.saves == 0);
    state.input(start, true);
    assert(!state.due(start + LearningCheckpoint::IdleUs - 1));
    assert(state.due(start + LearningCheckpoint::IdleUs));
    assert(state.save(store, false, start + LearningCheckpoint::IdleUs));
    assert(!state.dirty() && state.returned == 1);
    std::ifstream restored(path); const std::string value{std::istreambuf_iterator<char>(restored), {}};
    assert(value == "fixture-phrase");
    state.input(start + 3000000, true);
    for (std::uint64_t i = 4; i < 33; ++i) {
        state.input(start + i * 1000000, false);
        assert(!state.due(start + i * 1000000));
    }
    state.input(start + 33000000, false);
    assert(state.due(start + 33000000)); // Continuous input cannot postpone every checkpoint.
    store.fail = true;
    assert(!state.save(store, false, start + 33000000));
    assert(state.dirty() && state.failures == 1);
    assert(!state.due(start + 33000001));
    const auto attempts = state.attempts;
    assert(!state.save(store, false, start + 33000001));
    assert(state.attempts == attempts);
    store.fail = false;
    assert(state.due(start + 38000000));
    assert(state.save(store, false, start + 38000000));
    assert(state.save(store, true, start + 39000000)); // Drain native writes even when clean.
    assert(state.syncReturned == 1 && !state.dirty());
    std::filesystem::remove_all(directory);
}
