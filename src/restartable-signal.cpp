#include <atomic>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "common.h"

sem_t sem;
std::atomic_int child_tid = 0;
bool signal_got = false;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(gettid() == child_tid);
    signal_got = true;
}

void* thread_main(void*) {
    child_tid = gettid();
    // Make sure we don't get EINTR here
    ASSERT(sem_wait(&sem) == 0);
    ASSERT(signal_got);
    ASSERT(sem_post(&sem) == 0);
    return nullptr;
}

int main() {
    sigset_t full;
    sigfillset(&full);
    sem_init(&sem, false, 1);
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = signal_handler;
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);

    ASSERT(sem_wait(&sem) == 0);

    pthread_t thread;
    ASSERT(pthread_create(&thread, nullptr, thread_main, nullptr) == 0);

    // Wait for child to deadlock...
    usleep(100000);

    // Signal will land on child, sem_wait will not return EINTR as it is restartable
    tgkill(getpid(), child_tid, SIGUSR1);

    // Wait for child to receive...
    usleep(100000);

    ASSERT(sem_post(&sem) == 0);

    ASSERT(pthread_join(thread, nullptr) == 0);
    ASSERT(signal_got);
    return 0;
}