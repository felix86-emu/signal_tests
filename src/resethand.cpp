#include "common.h"

int count = 0;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    count++;
}

int main() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
    sa.sa_sigaction = signal_handler;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGURG, &sa, nullptr) == 0);
    raise(SIGURG);
    // reset to default, which is ignore
    raise(SIGURG);
    ASSERT(count == 1);
    return 0;
}