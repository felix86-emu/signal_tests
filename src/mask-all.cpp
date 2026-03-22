#include "common.h"

constexpr int mysig = 40;
int count = 0;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    if (count == 0) {
        ASSERT(sig == mysig);
    } else {
        ASSERT(sig == mysig + 1);
    }
    count++;
}

int main() {
    sigset_t full;
    sigfillset(&full);
    sigprocmask(SIG_BLOCK, &full, nullptr);
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = signal_handler;
    sigfillset(&sa.sa_mask);
    ASSERT(sigaction(mysig, &sa, nullptr) == 0);
    ASSERT(sigaction(mysig + 1, &sa, nullptr) == 0);

    raise(mysig);
    raise(mysig + 1);

    ASSERT(sigprocmask(SIG_UNBLOCK, &full, nullptr) == 0);

    ASSERT(count == 2);
    return 0;
}