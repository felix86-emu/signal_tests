#include <sys/ucontext.h>
#include "common.h"

#ifdef __x86_64__
int main() {
    // We don't care just pass
    return 0;
}
#else
u32 vcs, vss, vds, ves, vgs, vfs;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ucontext_t* uctx = (ucontext_t*)ctx;
    ASSERT(uctx->uc_mcontext.gregs[REG_ES] == ves);
    ASSERT(uctx->uc_mcontext.gregs[REG_CS] == vcs);
    ASSERT(uctx->uc_mcontext.gregs[REG_DS] == vds);
    ASSERT(uctx->uc_mcontext.gregs[REG_SS] == vss);
    ASSERT(uctx->uc_mcontext.gregs[REG_GS] == vgs);
    ASSERT(uctx->uc_mcontext.gregs[REG_FS] == vfs);
}

__attribute__((naked)) u32 get_gs() {
    asm(R"(
        mov eax, gs
        ret
    )");
}

__attribute__((naked)) u32 get_es() {
    asm(R"(
        mov eax, es
        ret
    )");
}

__attribute__((naked)) u32 get_ss() {
    asm(R"(
        mov eax, ss
        ret
    )");
}

__attribute__((naked)) u32 get_cs() {
    asm(R"(
        mov eax, cs
        ret
    )");
}

__attribute__((naked)) u32 get_fs() {
    asm(R"(
        mov eax, fs
        ret
    )");
}

__attribute__((naked)) u32 get_ds() {
    asm(R"(
        mov eax, ds
        ret
    )");
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, nullptr);
    vcs = get_cs();
    vds = get_ds();
    vss = get_ss();
    ves = get_es();
    vfs = get_fs();
    vgs = get_gs();
    raise(SIGUSR1);
}
#endif