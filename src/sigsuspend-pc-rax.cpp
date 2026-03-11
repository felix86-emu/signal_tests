#include <thread>
#include "common.h"

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    // When a signal interrupts a syscall the RIP will point to the instruction after the signal and RAX will be set to EINTR
    ucontext_t* uctx = (ucontext_t*)ctx;
    ASSERT(uctx->uc_mcontext.gregs[REG_RAX] == -EINTR);

    // Also check that previous instruction is a syscall
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
    int pid = getpid();
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);

    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);

    sigset_t block_all;
    sigfillset(&block_all);
    pthread_sigmask(SIG_BLOCK, &block_all, nullptr);

    std::thread t([pid]() {
        // Signals won't end up here because we inherited parents mask
        sigset_t my_mask;
        pthread_sigmask(SIG_BLOCK, nullptr, &my_mask);
        // Ensure signals are really blocked
        ASSERT(*(uint32_t*)&my_mask == (-1u & ~(1 << 31) & ~(1 << (SIGKILL - 1)) & ~(1 << (SIGSTOP - 1))));
        // Signal will be picked up by parent when sigsuspend happens
        kill(pid, SIGUSR1);
    });

    int ret = sigsuspend(&set);
    ASSERT(ret == -1);
    ASSERT(errno == EINTR);

    t.join();
    return 0;
}