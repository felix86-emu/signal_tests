#include "common.h"

int count = 0;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    count++;
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);

    sigset_t set;
    sigfillset(&set);
    ASSERT(sigprocmask(SIG_BLOCK, &set, nullptr) == 0);

    raise(SIGUSR1);
    raise(SIGUSR1);
    raise(SIGUSR1);
    raise(SIGUSR1);
    raise(SIGUSR1);

    ASSERT(count == 0);

    sigprocmask(SIG_UNBLOCK, &set, nullptr);

    ASSERT(count == 1);
    return 0;
}
