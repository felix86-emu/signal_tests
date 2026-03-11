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

    std::thread t([pid]() {
        sigset_t block_all;
        sigfillset(&block_all);
        pthread_sigmask(SIG_BLOCK, &block_all, nullptr);
        // Wait for parent to enter pause... No better way unfortunately
        usleep(500000);
        kill(pid, SIGUSR1);
    });

    int ret = pause();
    ASSERT(ret == -1);
    ASSERT(errno == EINTR);

    t.join();
    return 0;
}