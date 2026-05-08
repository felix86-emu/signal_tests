#include <sys/wait.h>
#include <syscall.h>
#include "common.h"

int child_main() {
    sigset_t mask;
    sigemptyset(&mask);
    sigsuspend(&mask);
    return 1;
}

int main() {
    sigset_t mask;
    sigfillset(&mask);
    ASSERT(sigprocmask(SIG_BLOCK, &mask, nullptr) == 0);
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = (decltype(sa.sa_sigaction))SIG_IGN;
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);
    sa.sa_sigaction = (decltype(sa.sa_sigaction))SIG_DFL; // default == ignore
    ASSERT(sigaction(SIGURG, &sa, nullptr) == 0);
    int pid = fork();
    if (pid == 0) {
        return child_main();
    }

    ASSERT(tgkill(pid, pid, SIGUSR1) == 0);
    usleep(10000);
    ASSERT(tgkill(pid, pid, SIGURG) == 0);
    usleep(100000);
    ASSERT(tgkill(pid, pid, SIGKILL) == 0);

    // Ensure SIGUSR1/SIGURG didn't kill it or make it exit (out of sigsuspend)
    // and instead SIGKILL did
    int status;
    int r = waitpid(pid, &status, 0);
    ASSERT(r == pid);
    ASSERT(WIFSIGNALED(status));
    ASSERT(WTERMSIG(status) == SIGKILL);
    return 0;
}
