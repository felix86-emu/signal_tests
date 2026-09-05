#include <string.h>
#include "common.h"

#ifdef __x86_64__
#define XSTATE_BV_OFFSET 512
#else
#define XSTATE_BV_OFFSET 624
#endif

bool handler_ran = false;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ucontext_t* uc = (ucontext_t*)ctx;
    u64* xstate_bv = (u64*)((u8*)uc->uc_mcontext.fpregs + XSTATE_BV_OFFSET);
    handler_ran = true;
    ASSERT(*xstate_bv & 2);
    *xstate_bv &= ~2ull;
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);

    u32 before[4] = {0xdead0000, 0xdead0001, 0xdead0002, 0xdead0003};
    u32 after[4];
    asm volatile("movups xmm0, %0" : : "m"(before));
    raise(SIGUSR1);
    asm volatile("movups %0, xmm0" : "=m"(after));
    ASSERT(handler_ran);
    for (int i = 0; i < 4; i++) {
        ASSERT(after[i] == 0);
    }
    return 0;
}
