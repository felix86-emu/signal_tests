#include <csignal>
#include "common.h"

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(sig == SIGSEGV);
    ASSERT(info->si_code == SEGV_MAPERR);
    ASSERT(info->si_addr == (void*)0x1000);
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

    volatile int* ptr = (int*)0x1000;
    *ptr = 0;
    return 1;
}