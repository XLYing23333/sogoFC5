#include "worker_runtime.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <poll.h>
#include <sys/prctl.h>
#include <sys/wait.h>
using namespace sogo::direct;
struct Backend {
    std::filesystem::path file;
    sogo::fc4::ShellView::Snapshot value;
    bool stall = false;
    void begin() { value.result.clear(); }
    bool key(std::uint8_t, std::uint32_t, bool release) {
        begin(); if (!release) value.result = "fixture-learning"; return true;
    }
    void reset() { value = {}; }
    void select(std::int32_t) {}
    bool previous() { return false; }
    bool next() { return false; }
    auto snapshot() const { return value; }
    void saveLearning(bool sync) {
        if (sync && stall) for (;;) pause();
        std::ofstream output(file); output << "fixture-learning";
        if (!output) throw std::runtime_error("fixture write failed");
    }
};
Message reply(int fd) {
    pollfd poller{fd, POLLIN, 0}; assert(poll(&poller, 1, 3000) == 1);
    Message message; assert(receive(fd, message) == 1); return message;
}
int main() {
    char name[] = "/tmp/sogo-worker-lifecycle-XXXXXX";
    auto* temporary = mkdtemp(name); assert(temporary);
    const auto root = std::filesystem::path(temporary);
    for (int mode = 0; mode < 5; ++mode) {
        const auto directory = root / std::to_string(mode);
        std::filesystem::create_directory(directory);
        int sockets[2]; assert(socketpair(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK, 0, sockets) == 0);
        const auto child = fork(); assert(child >= 0);
        if (!child) {
            close(sockets[0]); setenv("XDG_RUNTIME_DIR", directory.c_str(), 1);
            int result;
            {
                TerminationSignals termination;
                int argc = 1; char argument[] = "fixture-worker"; char* argv[]{argument, nullptr};
                QCoreApplication app(argc, argv);
                Backend backend{directory / "dictionary", {}, mode == 4};
                result = runWorker(app, backend, sockets[1], termination);
            }
            close(sockets[1]); _exit(result);
        }
        close(sockets[1]);
        assert(reply(sockets[0]).header.type == static_cast<std::uint32_t>(Command::Ready));
        assert(send(sockets[0], packet(Command::Focus, 1, 1)));
        auto key = packet(Command::Key, 1, 1, 1); key.sym = 'a';
        assert(send(sockets[0], key));
        assert(decode(reply(sockets[0]).body).result == "fixture-learning");
        if (mode == 0 || mode == 4) assert(send(sockets[0], packet(Command::Stop)));
        else if (mode == 1) { close(sockets[0]); sockets[0] = -1; }
        else if (mode == 2) assert(kill(child, SIGTERM) == 0);
        else {
            assert(send(sockets[0], packet(Command::Save)));
            key.sequence = 2; key.release = 1; assert(send(sockets[0], key));
            reply(sockets[0]); // Save is processed before this key reply.
            assert(std::filesystem::exists(directory / "dictionary"));
            assert(send(sockets[0], packet(Command::Stop)));
        }
        int status = 0; assert(waitpid(child, &status, 0) == child);
        if (sockets[0] >= 0) close(sockets[0]);
        if (mode == 4) { assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM); continue; }
        assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
        // Read with a fresh stream after process exit, never reuse live memory.
        std::ifstream input(directory / "dictionary");
        const std::string value{std::istreambuf_iterator<char>(input), {}};
        assert(value == "fixture-learning");
        std::ifstream metadata(directory / "sogo-fc5-direct/learning.json");
        const std::string report{std::istreambuf_iterator<char>(metadata), {}};
        assert(report.find("\"last_sync\":true") != std::string::npos);
        assert(report.find("fixture-learning") == std::string::npos);
    }
    std::filesystem::remove_all(root);
}
