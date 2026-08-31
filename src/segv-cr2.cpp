#include <string.h>
#include "common.h"

volatile int* bad_address = (int*)0x12340000;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ucontext_t* uc = (ucontext_t*)ctx;
#ifdef __x86_64__
    u64 cr2 = uc->uc_mcontext.gregs[REG_CR2];
#else
    u64 cr2 = uc->uc_mcontext.cr2;
#endif
    ASSERT(cr2 == (u64)(uintptr_t)bad_address);
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
