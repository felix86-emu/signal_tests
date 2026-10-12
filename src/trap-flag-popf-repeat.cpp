#include <csignal>
#include "common.h"

int count = 0;

bool __attribute__((naked)) get_tf() {
#ifdef __x86_64__
    asm(R"(
        pushf
        pop rax
        shr eax, 8
        and eax, 1
        ret
    )");
#else
    asm(R"(
        pushf
        pop eax
        shr eax, 8
        and eax, 1
        ret
    )");
#endif
}

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ucontext_t* uc = (ucontext_t*)ctx;
    u64 rip = uc->uc_mcontext.gregs[REG_RIP];
    ASSERT(sig == SIGTRAP);
    ASSERT(info->si_code == TRAP_TRACE);
    ASSERT(info->si_addr == (void*)rip);
    ASSERT(get_tf() == false);
    count++;
}

extern "C" void __attribute__((naked)) body() {
    asm(R"(
        .rept 4
        nop
        .endr
        ret
    )");
}

void __attribute__((naked)) run() {
#ifdef __x86_64__
    asm(R"(
        pushf
        or dword ptr [rsp], 0x100
        popf
        nop
        call body
        pushf
        and dword ptr [rsp], 0xfffffeff
        popf
        nop
        nop
        ret
    )");
#else
    asm(R"(
        pushf
        or dword ptr [esp], 0x100
        popf
        nop
        call body
        pushf
        and dword ptr [esp], 0xfffffeff
        popf
        nop
        nop
        ret
    )");
#endif
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGTRAP, &sa, nullptr) == 0);
    for (int i = 0; i < 8; i++) {
        count = 0;
        run();
        ASSERT(get_tf() == false);
        ASSERT(count == 10);
        for (int j = 0; j < 1000; j++) {
            body();
        }
        ASSERT(count == 10);
    }
    return 0;
}
