#include "common.h"

constexpr int mysig = 40;
int count = 0;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    if (count < 32) {
        ASSERT(sig == mysig);
        int i = count;
        ASSERT(info->si_value.sival_int == 0xCAFE + i);
    } else {
        ASSERT(sig == mysig + 1);
        int i = count - 32;
        ASSERT(info->si_value.sival_int == 0xCAFE + i);
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

    // Queue up 32 of mysig and 32 of mysig + 1
    // All the mysig will be handled before any of the mysig + 1
    // POSIX defines up to 32, Linux can queue way more by default
    for (int i = 0; i < 32; i++) {
        sigval val;
        val.sival_int = 0xCAFE + i;
        ASSERT(sigqueue(getpid(), mysig + 1, val) == 0);
        ASSERT(sigqueue(getpid(), mysig, val) == 0);
    }

    // Handle them all now and ensure order is correct
    sigprocmask(SIG_UNBLOCK, &full, nullptr);
    ASSERT(count == 64);
    return 0;
}