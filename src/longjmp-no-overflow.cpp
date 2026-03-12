#include <csignal>
#include <setjmp.h>
#include <sys/ucontext.h>
#include <unistd.h>
#include "common.h"

u64 count = 0;
jmp_buf outside;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    // Manually restore the mask since the longjmp will prevent it from being restored
    sigprocmask(SIG_SETMASK, &((ucontext_t*)ctx)->uc_sigmask, nullptr);
    longjmp(outside, 42);
}

int main() {
    u64 max = 128 * 1024;
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);
    int ret = setjmp(outside);

    // If an emulator was using the host stack for whatever reason per signal, after many signals
    // that don't return it could stack overflow, which is what we test here
    // If each signal was serviced inside the host signal handler, it would take up more than e.g. 512 bytes
    // per signal. At 128 * 1024 signals, this would overflow the host stack by quite a bit
    // Programs that use longjmp to exit signal handlers include the `dash` shell
    if (count++ < max) {
        raise(SIGUSR1);
    }

    ASSERT(count == max + 1);

    return 0;
}