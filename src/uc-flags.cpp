#include <string.h>
#include "common.h"

#define UC_FP_XSTATE 0x1
#define UC_SIGCONTEXT_SS 0x2
#define UC_STRICT_RESTORE_SS 0x4

bool handler_ran = false;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ucontext_t* uc = (ucontext_t*)ctx;
    handler_ran = true;
    ASSERT(uc->uc_flags & UC_FP_XSTATE);
#ifdef __x86_64__
    ASSERT(uc->uc_flags & UC_SIGCONTEXT_SS);
    ASSERT(uc->uc_flags & UC_STRICT_RESTORE_SS);
#endif
}

int main() {
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
