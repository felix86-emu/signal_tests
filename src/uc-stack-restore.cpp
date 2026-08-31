#include <string.h>
#include "common.h"

constexpr size_t ALTSZ = 1024 * 1024;
void* alt;
void* alt2;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ucontext_t* uc = (ucontext_t*)ctx;
    uc->uc_stack.ss_sp = alt2;
    uc->uc_stack.ss_size = ALTSZ;
    uc->uc_stack.ss_flags = 0;
}

int main() {
    alt = malloc(ALTSZ);
    alt2 = malloc(ALTSZ);
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

    stack_t q;
    ASSERT(sigaltstack(nullptr, &q) == 0);
    ASSERT(q.ss_sp == alt2);
    ASSERT(q.ss_size == ALTSZ);
    ASSERT(q.ss_flags == 0);
    return 0;
}
