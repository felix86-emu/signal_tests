#include <csignal>
#include "common.h"

int fault_count = 0;
u64 fault_rip = 0;
int fault_sig = 0;
int fault_trapno = 0;
int fault_err = 0;

extern "C" void do_far_jmp();
extern "C" char recovery[];

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ucontext_t* uc = (ucontext_t*)ctx;
    fault_count++;
    fault_sig = sig;
#ifdef __x86_64__
    fault_rip = uc->uc_mcontext.gregs[REG_RIP];
    fault_trapno = uc->uc_mcontext.gregs[REG_TRAPNO];
    fault_err = uc->uc_mcontext.gregs[REG_ERR];
    uc->uc_mcontext.gregs[REG_CSGSFS] = (uc->uc_mcontext.gregs[REG_CSGSFS] & ~0xFFFFLL) | 0x33;
    uc->uc_mcontext.gregs[REG_RIP] = (u64)recovery;
#else
    fault_rip = uc->uc_mcontext.gregs[REG_EIP];
    fault_trapno = uc->uc_mcontext.gregs[REG_TRAPNO];
    fault_err = uc->uc_mcontext.gregs[REG_ERR];
    uc->uc_mcontext.gregs[REG_CS] = 0x23;
    uc->uc_mcontext.gregs[REG_EIP] = (u32)(uintptr_t)recovery;
#endif
}

#ifdef __x86_64__
asm(R"(
.intel_syntax noprefix
.text
.globl do_far_jmp
.globl recovery
do_far_jmp:
    lea rax, [rip + target]
    push 0x07
    push rax
    ljmp [rsp]
target:
    nop
recovery:
    add rsp, 16
    ret
)");
#else
asm(R"(
.intel_syntax noprefix
.text
.globl do_far_jmp
.globl recovery
do_far_jmp:
    call 0f
0:  pop eax
    add eax, target - 0b
    push 0x07
    push eax
    ljmp [esp]
target:
    nop
recovery:
    add esp, 8
    ret
)");
#endif

int main() {
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGBUS, &sa, nullptr);

    do_far_jmp();

    ASSERT(fault_count == 1);
    ASSERT(fault_sig == SIGSEGV);
    // #GP is trapno 13
    ASSERT(fault_trapno == 13);

    return 0;
}
