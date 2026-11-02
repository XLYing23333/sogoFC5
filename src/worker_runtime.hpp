#pragma once
#include "editing.hpp"
#include "learning.hpp"
#include "shutdown.hpp"
#include <QCoreApplication>
#include <QSocketNotifier>
#include <QTimer>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <sys/stat.h>

namespace sogo::direct {
inline std::uint64_t monotonicUs() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
inline void publishLearning(const LearningCheckpoint& state, std::uint64_t duration,
                            bool sync) {
    const auto* runtime = std::getenv("XDG_RUNTIME_DIR");
    if (!runtime || !*runtime) return;
    const auto directory = std::filesystem::path(runtime) / "sogo-fc5-direct";
    std::error_code error; std::filesystem::create_directory(directory, error);
    struct stat info{};
    if (lstat(directory.c_str(), &info) || !S_ISDIR(info.st_mode) || info.st_uid != getuid()) return;
    chmod(directory.c_str(), 0700);
    auto temporary = (directory / "learning.json.XXXXXX").string();
    const int fd = mkstemp(temporary.data()); if (fd < 0) return;
    const auto data = std::string("{\"pid\":") + std::to_string(getpid()) +
        ",\"save_attempts\":" + std::to_string(state.attempts) +
        ",\"save_calls_returned\":" + std::to_string(state.returned) +
        ",\"sync_calls_returned\":" + std::to_string(state.syncReturned) +
        ",\"save_failures\":" + std::to_string(state.failures) +
        ",\"dirty\":" + (state.dirty() ? "true" : "false") +
        ",\"last_save_us\":" + std::to_string(duration) +
        ",\"last_sync\":" + (sync ? "true" : "false") +
        ",\"storage_verified\":false}\n";
    const auto count = write(fd, data.data(), data.size()); close(fd);
    if (count != static_cast<ssize_t>(data.size()) ||
        rename(temporary.c_str(), (directory / "learning.json").c_str())) unlink(temporary.c_str());
}

// Shared by the production worker and headless lifecycle tests. Backend owns
// native input/learning state; checkpoint policy never records input contents.
template<class Backend>
int runWorker(QCoreApplication& application, Backend& engine, int fd,
              const TerminationSignals& terminationSignals) {
    std::uint64_t generation = 0, context = 0;
    Editing editing; LearningCheckpoint learning;
    bool stopping = false;
    bool publishedDirty = false, lastSync = false;
    std::uint64_t lastSaveUs = 0;
    const auto stop = [&]() { stopping = true; application.quit(); };
    const auto save = [&](bool sync) {
        const auto begin = monotonicUs();
        learning.save(engine, sync, begin);
        lastSaveUs = monotonicUs() - begin; lastSync = sync;
        publishLearning(learning, lastSaveUs, lastSync);
        publishedDirty = learning.dirty();
    };
    publishLearning(learning, 0, false);
    QSocketNotifier termination(terminationSignals.fd(), QSocketNotifier::Read);
    QObject::connect(&termination, &QSocketNotifier::activated, &application,
        [&]() { if (terminationSignals.consume()) stop(); });
    QTimer checkpoint;
    checkpoint.setInterval(1000);
    QObject::connect(&checkpoint, &QTimer::timeout, &application, [&]() {
        if (!stopping && learning.due(monotonicUs())) save(false);
        else if (publishedDirty != learning.dirty()) {
            publishLearning(learning, lastSaveUs, lastSync);
            publishedDirty = learning.dirty();
        }
    });
    checkpoint.start();
    QSocketNotifier notifier(fd, QSocketNotifier::Read);
    QObject::connect(&notifier, &QSocketNotifier::activated, &application, [&]() {
        for (unsigned i = 0; i < 32 && !stopping; ++i) {
            Message message; const auto status = receive(fd, message);
            if (status == 0) return;
            if (status < 0) { stop(); return; }
            const auto& h = message.header; const auto command = static_cast<Command>(h.type);
            try {
                if (command == Command::Stop) { stop(); return; }
                if (command == Command::Save) { if (learning.dirty()) save(false); continue; }
                if (command == Command::Focus) {
                    engine.reset(); editing.reset(); generation = h.generation; context = h.context; continue;
                }
                if (command == Command::Blur) {
                    engine.reset(); editing.reset(); generation = context = 0;
                    if (learning.dirty()) save(false);
                    continue;
                }
                if (h.generation != generation || h.context != context) continue;
                const auto begin = monotonicUs(); auto outcome = editing.process(engine, h);
                learning.input(monotonicUs(), !outcome.value.result.empty());
                auto reply = h; reply.type = static_cast<std::uint32_t>(Command::Reply);
                reply.handled = outcome.flags; reply.engineUs = monotonicUs() - begin;
                if (!send(fd, reply, encode(outcome.value))) { stop(); return; }
            } catch (const std::exception&) { send(fd, packet(Command::Fault)); stop(); return; }
        }
    });
    int result = 1;
    if (terminationSignals.consume()) result = 0;
    else if (send(fd, packet(Command::Ready))) result = application.exec();
    // Native fsync and subsequent library cleanup stay in this process. A
    // stalled shutdown cannot leave an orphan worker running indefinitely.
    notifier.setEnabled(false); checkpoint.stop(); terminationSignals.boundCleanup();
    save(true);
    return result;
}
}
