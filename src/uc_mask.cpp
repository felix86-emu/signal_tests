#include <csignal>
#include <sys/ucontext.h>
#include <unistd.h>
#include "common.h"

bool all_ok = false;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ucontext_t* uc = (ucontext_t*)ctx;
    ASSERT(!sigismember(&uc->uc_sigmask, SIGURG));
    sigset_t current;
    sigprocmask(SIG_SETMASK, nullptr, &current);
    ASSERT(sigismember(&current, SIGURG));
}

int main(int argc, char* const* argv) {
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGURG);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);
    raise(SIGUSR1);
    sigset_t current;
    sigprocmask(SIG_SETMASK, nullptr, &current);
    ASSERT(!sigismember(&current, SIGURG));
}