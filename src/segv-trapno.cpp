#include <string.h>
#include "common.h"

volatile int* bad_address = (int*)0x12340000;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ucontext_t* uc = (ucontext_t*)ctx;
    ASSERT(info->si_code == SEGV_MAPERR);
    ASSERT(info->si_addr == (void*)bad_address);
    ASSERT(uc->uc_mcontext.gregs[REG_TRAPNO] == 14);
    ASSERT(uc->uc_mcontext.gregs[REG_ERR] == 0x6);
    exit(0);
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGSEGV, &sa, nullptr) == 0);
    *bad_address = 5;
    ASSERT(false);
    return 1;
}
