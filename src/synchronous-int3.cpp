#include <csignal>
#include <sys/ucontext.h>
#include "common.h"

__attribute__((naked)) void int3() {
    asm(R"(
        int3
        ret
    )");
}

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(sig == SIGTRAP);
    ASSERT(info->si_code == SI_KERNEL);
    ASSERT(info->si_addr == 0);
    ASSERT((u64)&int3 == ((ucontext_t*)ctx)->uc_mcontext.gregs[REG_RIP] - 1);
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
    ASSERT(sigaction(SIGTRAP, &sa, nullptr) == 0);

    int3();
    return 1;
}