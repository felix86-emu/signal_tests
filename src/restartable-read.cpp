#include <atomic>
#include <pthread.h>
#include <semaphore.h>
#include <syscall.h>
#include <unistd.h>
#include "common.h"

int pipedes[2];
std::atomic_int child_tid = 0;
bool signal_got = false;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(gettid() == child_tid);
    // Since this is a restartable syscall, the return address is the syscall instruction itself
    // RAX is not changed and has the original value, which is the syscall number
    ucontext_t* uctx = (ucontext_t*)ctx;
    u8* rip = (u8*)uctx->uc_mcontext.gregs[REG_RIP];
#ifdef __x86_64__
    ASSERT(*(rip) == 0x0f);
    ASSERT(*(rip + 1) == 0x05);
    ASSERT(uctx->uc_mcontext.gregs[REG_RAX] == SYS_read);
#else
    ASSERT(*(rip) == 0xcd);
    ASSERT(*(rip + 1) == 0x80);
    ASSERT(uctx->uc_mcontext.gregs[REG_RAX] == SYS_read);
#endif
    signal_got = true;
}

void* thread_main(void*) {
    child_tid = gettid();
    // Make sure we don't get EINTR here
    char buffer[16];
    ASSERT(read(pipedes[0], buffer, 16) == 16);
    return nullptr;
}

int main() {
    ASSERT(pipe(pipedes) == 0);

    sigset_t full;
    sigfillset(&full);
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = signal_handler;
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);

    pthread_t thread;
    ASSERT(pthread_create(&thread, nullptr, thread_main, nullptr) == 0);

    // Wait for child to deadlock...
    usleep(200000);

    // Signal will land on child, read will not return EINTR as it is restartable
    tgkill(getpid(), child_tid, SIGUSR1);

    // Wait for child to receive...
    usleep(100000);

    // Write to the pipe to allow the child to exit
    char data[16];
    ASSERT(write(pipedes[1], data, 16) == 16);

    ASSERT(pthread_join(thread, nullptr) == 0);
    ASSERT(signal_got);
    return 0;
}