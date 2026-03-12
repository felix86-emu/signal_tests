#include <csignal>
#include "common.h"

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(sig == SIGSEGV);
    ASSERT(info->si_code == SI_KERNEL);
    ASSERT(info->si_addr == 0);
    exit(0);
}

__attribute__((naked)) void hlt() {
    asm(R"(
        hlt
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

    hlt();
    return 1;
}