#include <csignal>
#include "common.h"

int count = 0;
bool rip_ok = true;
void* expected_rip = nullptr;

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
    count++;
    ucontext_t* uc = (ucontext_t*)ctx;
    u64 rip = uc->uc_mcontext.gregs[REG_RIP];
    ASSERT(info->si_code == TRAP_TRACE);
    ASSERT(info->si_addr == (void*)rip);
    ASSERT(uc->uc_mcontext.gregs[REG_EFL] & 0x100);
    ASSERT(uc->uc_mcontext.gregs[REG_ERR] == 0);
    ASSERT(uc->uc_mcontext.gregs[REG_TRAPNO] == 1);
    ASSERT(get_tf() == false);

    ASSERT(*(u8*)(rip - 1) == 0x90);

    if (count == 1) {
        // breaks on the first nop, RIP points to second for returning
        ASSERT(*(u8*)(rip - 2) == 0x9d);
        ASSERT(*(u8*)(rip - 1) == 0x90);
        ASSERT(*(u8*)(rip) == 0x90);
    }

    if (rip != (u64)expected_rip + count - 1) {
        rip_ok = false;
    }

    if (count >= 10) {
        uc->uc_mcontext.gregs[REG_EFL] &= ~0x100;
    }
}

void __attribute__((naked)) run(void* ptr) {
#ifdef __x86_64__
    asm(R"(
        lea rax, [rip + 1f]
        mov qword ptr[rdi], rax
        pushf
        or dword ptr [rsp], 0x100
        popf
        nop
        1:
        .rept 9
        nop
        .endr
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
        .rept 9
        nop
        .endr
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
    ASSERT(rip_ok);
    ASSERT(count == 10);
    return 0;
}
