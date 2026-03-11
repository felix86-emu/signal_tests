#include <csignal>
#include <sys/ucontext.h>
#include "common.h"
#define SS_AUTODISARM (1U << 31)

void* new_stack;
int signal_count = 0;

#ifndef REG_RSP
#define REG_RSP REG_ESP
#endif

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ucontext_t* uctx = (ucontext_t*)ctx;
    stack_t old_stack;
    sigaltstack(nullptr, &old_stack);
    printf("Got signal %d %lx %lx %lx\n", sig, uctx->uc_mcontext.gregs[REG_RSP], uctx->uc_stack.ss_sp, new_stack);
    signal_count++;
    bool was_on_alt_stack = uctx->uc_mcontext.gregs[REG_RSP] > (u64)new_stack && uctx->uc_mcontext.gregs[REG_RSP] - (u64)new_stack <= 1024 * 1024;
    if (signal_count == 1) {
        ASSERT(old_stack.ss_sp == nullptr);
        ASSERT(uctx->uc_stack.ss_sp == nullptr);
        ASSERT(was_on_alt_stack);
    } else if (signal_count == 2) {
        ASSERT(old_stack.ss_sp == nullptr);
        ASSERT(uctx->uc_stack.ss_sp == nullptr);
        ASSERT(was_on_alt_stack);
    } else if (signal_count == 3) {
        ASSERT(old_stack.ss_sp == nullptr);
        ASSERT(uctx->uc_stack.ss_sp == new_stack);
        ASSERT(!was_on_alt_stack);
    } else {
        ASSERT(false);
    }
}

int main() {
    new_stack = malloc(1024 * 1024);
    stack_t stack;
    stack.ss_sp = (u8*)new_stack;
    stack.ss_size = 1024 * 1024;
    stack.ss_flags = SS_AUTODISARM;
    ASSERT(sigaltstack(&stack, nullptr) == 0);
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGURG, &sa, nullptr);
    sigaction(SIGPWR, &sa, nullptr);
    sigaction(SIGUSR1, &sa, nullptr);

    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, nullptr);
    raise(SIGURG);
    raise(SIGPWR);
    raise(SIGUSR1);
    sigprocmask(SIG_UNBLOCK, &set, nullptr);
    return 0;
}