#include <sys/wait.h>
#include <syscall.h>
#include "common.h"

int parent_pid = 0;
int uid = 0;
constexpr u64 magic = 0xabcdef0cafe1234;
int mysig = SIGRTMIN + 10;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(sig == mysig);
    ASSERT(info->si_code == -0xcafe);
    ASSERT(info->si_pid == parent_pid);
    ASSERT(info->si_uid == uid);
    ASSERT(info->si_value.sival_ptr == (void*)magic);

    // See sigsuspend test
    ucontext_t* uctx = (ucontext_t*)ctx;
    ASSERT(uctx->uc_mcontext.gregs[REG_RAX] == -EINTR);

    u8* rip = (u8*)uctx->uc_mcontext.gregs[REG_RIP];
#ifdef __x86_64__
    ASSERT(*(rip - 2) == 0x0f);
    ASSERT(*(rip - 1) == 0x05);
#else
    ASSERT(*(rip - 2) == 0xcd);
    ASSERT(*(rip - 1) == 0x80);
#endif
}

int main() {
    uid = geteuid();
    parent_pid = getpid();
    sigset_t full;
    sigfillset(&full);
    sigprocmask(SIG_BLOCK, &full, nullptr);

    int pid = fork();
    if (pid == 0) {
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = signal_handler;
        sigemptyset(&sa.sa_mask);
        sigaction(mysig, &sa, nullptr);
        sigset_t wait_me;
        sigfillset(&wait_me);
        sigdelset(&wait_me, mysig);
        int ret = sigsuspend(&wait_me);
        ASSERT(ret == -1);
        ASSERT(errno == EINTR);
        return 0;
    } else {
        siginfo_t custom_info = {};
        custom_info.si_code = -0xcafe; // random, must be < 0
        custom_info.si_pid = parent_pid;
        custom_info.si_uid = uid;
        custom_info.si_value.sival_ptr = (void*)magic;
        int val = syscall(SYS_rt_sigqueueinfo, pid, mysig, &custom_info);

        int status = 0;
        pid_t w = waitpid(pid, &status, 0);

        if (w == -1) {
            return 1;
        }

        ASSERT(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    }
}