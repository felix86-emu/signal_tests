#include <csignal>
#include "common.h"

bool first_time = true;
bool inside_signal_handler = false;
int count = 0;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(!inside_signal_handler);
    inside_signal_handler = true;
    count++;
    if (count == 1) {
        // Must happen only after signal handler returns
        raise(SIGUSR1);
    }
    inside_signal_handler = false;
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, nullptr);
    raise(SIGUSR1);
    ASSERT(count == 2);
    return 0;
}