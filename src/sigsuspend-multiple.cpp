#include <syscall.h>
#include "common.h"

int count = 0;

// Only one signal happens per sigsuspend
void signal_handler(int sig, siginfo_t* info, void* ctx) {
    count++;
    ucontext_t* uctx = (ucontext_t*)ctx;
    ASSERT(uctx->uc_mcontext.gregs[REG_RAX] == -EINTR);

    u8* rip = (u8*)uctx->uc_mcontext.gregs[REG_RIP];
#ifdef __x86_64__
    ASSERT(*(rip - 2) == 0x0f);
    ASSERT(*(rip - 1) == 0x05);
#else
    ASSERT(*(rip - 2) == 0xcd);
    ASSERT(*(rip - 1) == 0x80);
#endif
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigfillset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);
    ASSERT(sigaction(SIGUSR2, &sa, nullptr) == 0);
    ASSERT(sigaction(SIGURG, &sa, nullptr) == 0);
    ASSERT(sigaction(40, &sa, nullptr) == 0);

    u64 full = -1ull;
    u64 zero = 0;
    ASSERT(syscall(SYS_rt_sigprocmask, SIG_BLOCK, &full, nullptr, sizeof(u64)) == 0);
    ASSERT(raise(SIGUSR1) == 0);
    ASSERT(raise(SIGUSR2) == 0);
    ASSERT(raise(SIGURG) == 0);
    ASSERT(raise(40) == 0);
    syscall(SYS_rt_sigsuspend, &zero, sizeof(u64));

    ASSERT(count == 1);
    return 0;
}