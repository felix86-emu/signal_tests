#include <string.h>
#include "common.h"

bool handler_ran = false;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ucontext_t* uc = (ucontext_t*)ctx;
    handler_ran = true;
#ifdef __x86_64__
    u64 csgsfs = uc->uc_mcontext.gregs[REG_CSGSFS];
    ASSERT((csgsfs & 0xFFFF) == 0x33);
    ASSERT(((csgsfs >> 16) & 0xFFFF) == 0);
    ASSERT(((csgsfs >> 32) & 0xFFFF) == 0);
    ASSERT(((csgsfs >> 48) & 0xFFFF) == 0x2b);
#else
    ASSERT(uc->uc_mcontext.gregs[REG_CS] == 0x23);
    ASSERT(uc->uc_mcontext.gregs[REG_SS] == 0x2b);
    ASSERT(uc->uc_mcontext.gregs[REG_DS] == 0x2b);
    ASSERT(uc->uc_mcontext.gregs[REG_ES] == 0x2b);
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
