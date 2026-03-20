#include <csignal>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include "common.h"

int child_tid = 0;
int parent_tid = 0;
bool is_ok = false;
bool is_ok2 = false;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(child_tid == gettid());
    is_ok = true;
}

void signal_handler2(int sig, siginfo_t* info, void* ctx) {
    ASSERT(parent_tid == gettid());
    is_ok2 = true;
}

void* thread_main(void* args) {
    child_tid = gettid();
    struct sigaction old_sa;
    ASSERT(sigaction(SIGUSR1, nullptr, &old_sa) == 0);
    ASSERT(old_sa.sa_sigaction == signal_handler);
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = signal_handler2;
    sigemptyset(&sa.sa_mask);
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

    pthread_t my_thread;
    ASSERT(pthread_create(&my_thread, nullptr, thread_main, nullptr) == 0);
    ASSERT(pthread_sigmask(SIG_BLOCK, &full, nullptr) == 0);
    ASSERT(pthread_join(my_thread, nullptr) == 0);

    struct sigaction old_sa;
    ASSERT(sigaction(SIGUSR2, nullptr, &old_sa) == 0);
    ASSERT(old_sa.sa_sigaction == signal_handler2);
    ASSERT(pthread_sigmask(SIG_UNBLOCK, &full, nullptr) == 0);
    raise(SIGUSR2);

    ASSERT(is_ok);
    ASSERT(is_ok2);
    return 0;
}