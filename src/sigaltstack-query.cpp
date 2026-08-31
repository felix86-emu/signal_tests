#include <string.h>
#include "common.h"

constexpr size_t ALTSZ = 1024 * 1024;
void* alt;
bool handler_ran = false;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    handler_ran = true;
    stack_t q;
    ASSERT(sigaltstack(nullptr, &q) == 0);
    ASSERT(q.ss_sp == alt);
    ASSERT(q.ss_size == ALTSZ);
    ASSERT(q.ss_flags == SS_ONSTACK);
}

int main() {
    alt = malloc(ALTSZ);

    stack_t none;
    ASSERT(sigaltstack(nullptr, &none) == 0);
    ASSERT(none.ss_sp == nullptr);
    ASSERT(none.ss_size == 0);
    ASSERT(none.ss_flags == SS_DISABLE);

    stack_t ss;
    ss.ss_sp = alt;
    ss.ss_size = ALTSZ;
    ss.ss_flags = 0;
    ASSERT(sigaltstack(&ss, nullptr) == 0);

    stack_t q;
    ASSERT(sigaltstack(nullptr, &q) == 0);
    ASSERT(q.ss_sp == alt);
    ASSERT(q.ss_size == ALTSZ);
    ASSERT(q.ss_flags == 0);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);
    raise(SIGUSR1);
    ASSERT(handler_ran);

    ASSERT(sigaltstack(nullptr, &q) == 0);
    ASSERT(q.ss_flags == 0);

    stack_t dis;
    dis.ss_sp = alt;
    dis.ss_size = ALTSZ;
    dis.ss_flags = SS_DISABLE;
    ASSERT(sigaltstack(&dis, nullptr) == 0);
    ASSERT(sigaltstack(nullptr, &q) == 0);
    ASSERT(q.ss_sp == nullptr);
    ASSERT(q.ss_size == 0);
    ASSERT(q.ss_flags == SS_DISABLE);
    return 0;
}
