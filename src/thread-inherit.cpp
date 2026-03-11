#include <csignal>
#include <thread>
#include <sched.h>
#include <sys/syscall.h>
#include "common.h"

int main() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGURG);
    sigaddset(&set, SIGSEGV);
    sigaddset(&set, SIGBUS);
    sigaddset(&set, SIGILL);
    ASSERT(sigprocmask(SIG_BLOCK, &set, nullptr) == 0);
    std::thread thread([]() {
        sigset_t old_mask;
        ASSERT(sigprocmask(SIG_SETMASK, nullptr, &old_mask) == 0);
        ASSERT(sigismember(&old_mask, SIGUSR1));
        ASSERT(sigismember(&old_mask, SIGURG));
        ASSERT(sigismember(&old_mask, SIGSEGV));
        ASSERT(sigismember(&old_mask, SIGBUS));
        ASSERT(sigismember(&old_mask, SIGILL));
    });

    thread.join();
    sigset_t old_mask;
    ASSERT(sigprocmask(SIG_SETMASK, nullptr, &old_mask) == 0);
    ASSERT(sigismember(&old_mask, SIGUSR1));
    ASSERT(sigismember(&old_mask, SIGURG));
    ASSERT(sigismember(&old_mask, SIGSEGV));
    ASSERT(sigismember(&old_mask, SIGBUS));
    ASSERT(sigismember(&old_mask, SIGILL));
    return 0;
}