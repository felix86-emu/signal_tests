#include <csignal>
#include <sys/ucontext.h>
#include "common.h"

bool sigurg_ok = false;
bool sigpwr_ok = false;
void* new_stack;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    if (sig == SIGURG) {
        sigurg_ok = true;
    } else if (sig == SIGPWR) {
        sigpwr_ok = true;
    }
    printf("Got signal %d\n", sig);
}

int main() {
    new_stack = malloc(1024 * 1024);
    stack_t stack;
    stack.ss_sp = (u8*)new_stack;
    stack.ss_size = 1024 * 1024;
    stack.ss_flags = 0;
    ASSERT(sigaltstack(&stack, nullptr) == 0);
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGURG, &sa, nullptr);
    sigaction(SIGPWR, &sa, nullptr);

    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, nullptr);
    raise(SIGURG);
    raise(SIGPWR);
    sigprocmask(SIG_UNBLOCK, &set, nullptr);

    if (sigurg_ok && sigpwr_ok) {
        return 0;
    }

    return 1;
}