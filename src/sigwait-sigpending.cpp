#include <string.h>
#include <time.h>
#include "common.h"

int main() {
    int rtsig = SIGRTMIN + 3;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, rtsig);
    ASSERT(sigprocmask(SIG_BLOCK, &set, nullptr) == 0);

    union sigval v;
    v.sival_int = 0x1234;
    ASSERT(sigqueue(getpid(), rtsig, v) == 0);

    sigset_t pending;
    sigemptyset(&pending);
    ASSERT(sigpending(&pending) == 0);
    ASSERT(sigismember(&pending, rtsig));
    ASSERT(!sigismember(&pending, SIGUSR1));

    siginfo_t si;
    memset(&si, 0xAA, sizeof(si));
    int r = sigwaitinfo(&set, &si);
    ASSERT(r == rtsig);
    ASSERT(si.si_signo == rtsig);
    ASSERT(si.si_code == SI_QUEUE);
    ASSERT(si.si_value.sival_int == 0x1234);
    ASSERT(si.si_pid == getpid());

    ASSERT(sigpending(&pending) == 0);
    ASSERT(!sigismember(&pending, rtsig));

    struct timespec ts = {0, 20 * 1000 * 1000};
    memset(&si, 0xAA, sizeof(si));
    errno = 0;
    r = sigtimedwait(&set, &si, &ts);
    ASSERT(r == -1);
    ASSERT(errno == EAGAIN);
    return 0;
}
