#include <csignal>
#include "common.h"

int count = 0;
u64 rips[16];
void* expected_rip = nullptr;
u64 last_efl = 0;

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
    if (count < 16) {
        rips[count] = rip;
    }
    last_efl = uc->uc_mcontext.gregs[REG_EFL];
    count++;
    if (count >= 16) {
        uc->uc_mcontext.gregs[REG_EFL] &= ~0x100;
    }
}

void __attribute__((naked)) run(void* ptr) {
#ifdef __x86_64__
    asm(R"(
        lea rax, [rip + 1f]
        mov qword ptr [rdi], rax
        pushf
        or dword ptr [rsp], 0x100
        popf
        nop
        1:
        .rept 4
        nop
        .endr
        pushf
        and dword ptr [rsp], 0xfffffeff
        popf
        nop
        nop
        ret
    )");
#else
    asm(R"(
        call 0f
        0: pop eax
        add eax, 1f - 0b
        mov edx, [esp + 4]
        mov dword ptr [edx], eax
        pushf
        or dword ptr [esp], 0x100
        popf
        nop
        1:
        .rept 4
        nop
        .endr
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
    run(&expected_rip);
    ASSERT(get_tf() == false);
    ASSERT(count == 8);
    u64 base = (u64)expected_rip;
    for (int i = 0; i < 5; i++) {
        ASSERT(rips[i] == base + i);
    }
    ASSERT(rips[5] == base + 5);
    ASSERT(rips[6] == base + 12);
    ASSERT(rips[7] == base + 13);
    ASSERT((last_efl & 0x100) == 0);
    return 0;
}
