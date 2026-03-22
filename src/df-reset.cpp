#include "common.h"

__attribute__((naked)) bool get_df() {
#ifdef __x86_64__
    asm(R"(
        pushf
        pop  rax
        shr  eax, 10
        and  eax, 1
        ret
    )");
#else
    asm(R"(
        pushf
        pop  eax
        shr  eax, 10
        and  eax, 1
        ret
    )");
#endif
}

__attribute__((naked)) void set_df(bool val) {
#ifdef __x86_64__
    asm(R"(
        pushf
        pop rax
        shl edi, 10
        or rax, rdi
        push rax
        popf
        ret
    )");
#else
    asm(R"(
        pushf
        pop eax
        mov ecx, [esp + 4]
        shl ecx, 10
        or eax, ecx
        push eax
        popf
        ret
    )");
#endif
}

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    bool df = get_df();
    ASSERT(!df);
    ASSERT((((ucontext_t*)ctx)->uc_mcontext.gregs[REG_EFL] >> 10) & 1);
    ((ucontext_t*)ctx)->uc_mcontext.gregs[REG_EFL] &= ~(1ull << 10);
}

int main() {
    set_df(true);
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = signal_handler;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);
    raise(SIGUSR1);
    bool df = get_df();
    ASSERT(!df);
    return 0;
}