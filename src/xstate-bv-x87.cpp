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
    ASSERT(*xstate_bv & 1);
    *xstate_bv &= ~1ull;
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);

    u16 cw = 0x027f;
    u32 env[7];
    asm volatile("fninit; fldcw %0; fld1" : : "m"(cw));
    raise(SIGUSR1);
    asm volatile("fnstenv %0" : "=m"(env));
    ASSERT(handler_ran);
    ASSERT((env[0] & 0xffff) == 0x037f);
    ASSERT((env[1] & 0xffff) == 0);
    ASSERT((env[2] & 0xffff) == 0xffff);
    return 0;
}
