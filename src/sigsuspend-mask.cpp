#include <syscall.h>
#include "common.h"

bool ok = false;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ok = true;
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
    // Signal happened during sigsuspend, but the mask here isn't the sigsuspend mask but the thread mask
    u64 full = -1ull;
    u64 kill_bit = 1ull << (SIGKILL - 1);
    u64 stop_bit = 1ull << (SIGSTOP - 1);
    ASSERT(uctx->uc_sigmask.__val[0] == (full & ~(kill_bit | stop_bit)));
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigfillset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);

    u64 full = -1ull;
    u64 zero = 0;
    ASSERT(syscall(SYS_rt_sigprocmask, SIG_BLOCK, &full, nullptr, sizeof(u64)) == 0);
    ASSERT(raise(SIGUSR1) == 0);
    syscall(SYS_rt_sigsuspend, &zero, sizeof(u64));

    ASSERT(ok);
    return 0;
}