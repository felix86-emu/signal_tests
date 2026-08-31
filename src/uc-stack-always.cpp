#include <string.h>
#include "common.h"

constexpr size_t ALTSZ = 1024 * 1024;
void* alt;
int count = 0;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ucontext_t* uc = (ucontext_t*)ctx;
    count++;
    ASSERT(uc->uc_stack.ss_sp == alt);
    ASSERT(uc->uc_stack.ss_size == ALTSZ);
    ASSERT(uc->uc_stack.ss_flags == 0);
}

int main() {
    alt = malloc(ALTSZ);
    stack_t ss;
    ss.ss_sp = alt;
    ss.ss_size = ALTSZ;
    ss.ss_flags = 0;
    ASSERT(sigaltstack(&ss, nullptr) == 0);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);
    raise(SIGUSR1);

    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);
    raise(SIGUSR1);

    ASSERT(count == 2);
    return 0;
}
