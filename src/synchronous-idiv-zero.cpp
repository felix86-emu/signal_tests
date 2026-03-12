#include <csignal>
#include "common.h"

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(sig == SIGFPE);
    ASSERT(info->si_code == FPE_INTDIV);
    ASSERT(info->si_addr == (void*)((ucontext_t*)ctx)->uc_mcontext.gregs[REG_RIP]);
    exit(0);
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGSEGV, &sa, nullptr) == 0);
    ASSERT(sigaction(SIGBUS, &sa, nullptr) == 0);
    ASSERT(sigaction(SIGILL, &sa, nullptr) == 0);
    ASSERT(sigaction(SIGFPE, &sa, nullptr) == 0);

    volatile int zero = 0;
    volatile int one = 1;
    volatile int result = one / zero;
    return 1;
}