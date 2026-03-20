#include <emmintrin.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "common.h"

int now_servicing = 34;
int serviced = 0;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(now_servicing == sig);
    now_servicing++;
    serviced++;
}

int main() {
    void* shared_mem = mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    u32* lock = (u32*)shared_mem;
    *lock = 1;

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = signal_handler;
    // Block all so we don't get signal inside signal handler
    sigfillset(&sa.sa_mask);
    for (int i = 34; i < 40; i++) {
        ASSERT(sigaction(i, &sa, nullptr) == 0);
    }

    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_BLOCK, &set, nullptr);
    int pid = fork();
    if (pid == 0) {
        // Wait for parent to send us all the signals
        while (__atomic_exchange_n(lock, 1, __ATOMIC_SEQ_CST))
            _mm_pause();

        sigset_t set;
        sigemptyset(&set);
        // After sigsuspend finishes, all signals are blocked again so only one is serviced
        ASSERT(sigsuspend(&set) == -1);
        ASSERT(errno == EINTR);
        ASSERT(serviced == 1);
        ASSERT(now_servicing == 35);
    } else {
        // Send a bunch, but only 34 is served
        ASSERT(kill(pid, 36) == 0);
        ASSERT(kill(pid, 35) == 0);
        ASSERT(kill(pid, 34) == 0);
        ASSERT(kill(pid, 37) == 0);
        ASSERT(kill(pid, 39) == 0);
        ASSERT(kill(pid, 38) == 0);

        // Allow child to start servicing
        __atomic_store_n(lock, 0, __ATOMIC_SEQ_CST);

        int status = 0;
        pid_t w = waitpid(pid, &status, 0);

        if (w == -1) {
            return 1;
        }

        ASSERT(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    }
}