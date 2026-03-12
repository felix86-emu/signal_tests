#include <csignal>
#include <sys/ucontext.h>
#include "common.h"

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(sig == SIGILL);
    ASSERT(info->si_code == ILL_ILLOPN);
    ASSERT(info->si_addr == (void*)((ucontext_t*)ctx)->uc_mcontext.gregs[REG_RIP]);
    exit(0);
}

__attribute__((naked)) void ud2() {
    asm(R"(
        ud2
        ret
    )");
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

    ud2();
    return 1;
}