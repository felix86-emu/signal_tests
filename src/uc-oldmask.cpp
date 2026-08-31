#include <string.h>
#include "common.h"

bool handler_ran = false;
u64 expected_mask = 0;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ucontext_t* uc = (ucontext_t*)ctx;
    handler_ran = true;
#ifdef __x86_64__
    u64 oldmask = uc->uc_mcontext.gregs[REG_OLDMASK];
#else
    u64 oldmask = uc->uc_mcontext.oldmask;
#endif
    ASSERT(oldmask == expected_mask);
}

int main() {
    sigset_t blocked;
    sigemptyset(&blocked);
    sigaddset(&blocked, SIGURG);
    sigaddset(&blocked, SIGVTALRM);
    ASSERT(sigprocmask(SIG_BLOCK, &blocked, nullptr) == 0);
    expected_mask = (1ull << (SIGURG - 1)) | (1ull << (SIGVTALRM - 1));

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);
    raise(SIGUSR1);
    ASSERT(handler_ran);
    return 0;
}
