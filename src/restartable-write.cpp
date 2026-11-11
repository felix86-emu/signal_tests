#include <atomic>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <syscall.h>
#include <unistd.h>
#include "common.h"

int pipedes[2];
std::atomic_int child_tid = 0;
std::atomic_int signals_handled = 0;
std::atomic_bool signal_got = false;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(gettid() == child_tid);
    // Since this is a restartable syscall, the return address is the syscall instruction itself
    // RAX is not changed and has the original value, which is the syscall number
    ucontext_t* uctx = (ucontext_t*)ctx;
    u8* rip = (u8*)uctx->uc_mcontext.gregs[REG_RIP];
#ifdef __x86_64__
    bool at_syscall = *(rip) == 0x0f && *(rip + 1) == 0x05;
#else
    bool at_syscall = *(rip) == 0xcd && *(rip + 1) == 0x80;
#endif
    if (at_syscall && uctx->uc_mcontext.gregs[REG_RAX] == SYS_write) {
        signal_got = true;
    }
    signals_handled++;
}

void* thread_main(void*) {
    child_tid = gettid();
    // Make sure we don't get EINTR here
    char buffer[16];
    ASSERT(write(pipedes[1], buffer, 16) == 16);
    return nullptr;
}

int main() {
    char buffer[4096];
    ASSERT(pipe(pipedes) == 0);

    // Fill the pipe so that subsequent writes will block
    ASSERT(fcntl(pipedes[1], F_SETPIPE_SZ, 4096) == 4096);
    ASSERT(write(pipedes[1], buffer, 4096) == 4096);

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = signal_handler;
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);

    pthread_t thread;
    ASSERT(pthread_create(&thread, nullptr, thread_main, nullptr) == 0);

    for (int attempt = 0; attempt < 3000 && !signal_got; attempt++) {
        if (child_tid == 0) {
            usleep(10000);
            continue;
        }

        int handled = signals_handled;
        // Signal will land on child, write will not return EINTR as it is restartable
        ASSERT(tgkill(getpid(), child_tid, SIGUSR1) == 0);
        for (int wait = 0; wait < 1000 && signals_handled == handled; wait++) {
            usleep(1000);
        }

        if (!signal_got) {
            usleep(10000);
        }
    }

    ASSERT(signal_got);

    // Read enough from the pipe to allow the child to exit
    ASSERT(read(pipedes[0], buffer, 4096) == 4096);

    ASSERT(pthread_join(thread, nullptr) == 0);
    return 0;
}
