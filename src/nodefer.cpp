#include "common.h"

bool finished = false;
int count = 0;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(!finished);
    if (count < 5) {
        count++;
        raise(SIGUSR1);
    }
    finished = true;
}

int main() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO | SA_NODEFER;
    sa.sa_sigaction = signal_handler;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);
    raise(SIGUSR1);
    ASSERT(count == 5);
    return 0;
}