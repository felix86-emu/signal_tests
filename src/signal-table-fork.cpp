#include <csignal>
#include <pthread.h>
#include <sched.h>
#include <sys/wait.h>
#include <unistd.h>
#include "common.h"

int child_tid = 0;
int parent_tid = 0;
bool is_ok = false;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(child_tid == gettid());
    is_ok = true;
}

void* thread_main(void* args) {
    child_tid = gettid();
    struct sigaction old_sa;
    ASSERT(sigaction(SIGUSR1, nullptr, &old_sa) == 0);
    ASSERT(old_sa.sa_sigaction == signal_handler);
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = signal_handler;
    sigemptyset(&sa.sa_mask);
    // Not propagated to parent
    ASSERT(sigaction(SIGUSR2, &sa, nullptr) == 0);
    raise(SIGUSR1);
    return nullptr;
}

// Checks that threads share the same signal table and a signal handler changing in one thread
// changes it in all
int main() {
    parent_tid = gettid();
    sigset_t full;
    sigfillset(&full);

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = signal_handler;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);

    child_tid = fork();
    if (child_tid == 0) {
        thread_main(nullptr);
        ASSERT(is_ok);
        exit(0);
    } else {
        int status = 0;
        pid_t w = waitpid(child_tid, &status, 0);

        if (w == -1) {
            return 1;
        }

        ASSERT(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    }

    struct sigaction old_sa;
    ASSERT(sigaction(SIGUSR2, nullptr, &old_sa) == 0);
    ASSERT(old_sa.sa_sigaction == (void*)SIG_DFL);
    return 0;
}