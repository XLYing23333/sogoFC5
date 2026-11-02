#pragma once
#include <cerrno>
#include <csignal>
#include <stdexcept>
#include <initializer_list>
#include <sys/signalfd.h>
#include <unistd.h>

namespace sogo::direct {
// Construct before Qt and vendor threads, so all threads inherit the mask.
// Signal delivery becomes ordinary event-loop work, never a vendor call from
// an asynchronous signal handler.
class TerminationSignals {
public:
    TerminationSignals() {
        sigset_t mask; sigemptyset(&mask);
        for (int signal : {SIGTERM, SIGINT, SIGHUP}) sigaddset(&mask, signal);
        if (sigprocmask(SIG_BLOCK, &mask, &previous_))
            throw std::runtime_error("cannot block termination signals");
        fd_ = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
        if (fd_ < 0) {
            sigprocmask(SIG_SETMASK, &previous_, nullptr);
            throw std::runtime_error("cannot create termination signal descriptor");
        }
    }
    ~TerminationSignals() {
        alarm(0);
        close(fd_);
        sigprocmask(SIG_SETMASK, &previous_, nullptr);
    }
    TerminationSignals(const TerminationSignals&) = delete;
    TerminationSignals& operator=(const TerminationSignals&) = delete;
    int fd() const { return fd_; }
    bool consume() const {
        signalfd_siginfo info{};
        bool received = false;
        for (;;) {
            const auto count = read(fd_, &info, sizeof(info));
            if (count == static_cast<ssize_t>(sizeof(info))) received = true;
            else if (count < 0 && errno == EINTR) continue;
            else return received;
        }
    }
    void boundCleanup(unsigned seconds = 3) const {
        struct sigaction action{}; action.sa_handler = SIG_DFL;
        sigemptyset(&action.sa_mask); sigaction(SIGALRM, &action, nullptr);
        sigset_t alarmMask; sigemptyset(&alarmMask); sigaddset(&alarmMask, SIGALRM);
        sigprocmask(SIG_UNBLOCK, &alarmMask, nullptr);
        alarm(seconds);
    }
private:
    int fd_ = -1;
    sigset_t previous_{};
};
}
