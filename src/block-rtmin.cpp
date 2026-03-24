#include <syscall.h>
#include "common.h"

// sigprocmask seems to work similar to pthread_sigmask, which disallows blocking RT_MIN and RT1
// But we need to ensure that an emulator can actually block them if necessary
int main() {
    u64 full = -1ull;
    ASSERT(syscall(SYS_rt_sigprocmask, SIG_BLOCK, &full, nullptr, 8) == 0);

    u64 old;
    ASSERT(syscall(SYS_rt_sigprocmask, SIG_SETMASK, nullptr, &old, 8) == 0);

    u64 kill_bit = 1ull << (SIGKILL - 1);
    u64 stop_bit = 1ull << (SIGSTOP - 1);
    ASSERT(old == (full & ~(kill_bit | stop_bit)));

    return 0;
}