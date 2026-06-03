#include <csignal>
#include <sys/mman.h>
#include <sys/ucontext.h>
#include "common.h"

u64 expected_rip = -1ull;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(sig == SIGILL);
    ASSERT(info->si_code == ILL_ILLOPN);
    ASSERT(info->si_addr == (void*)((ucontext_t*)ctx)->uc_mcontext.gregs[REG_RIP]);
    ASSERT(((ucontext_t*)ctx)->uc_mcontext.gregs[REG_RIP] == expected_rip);
    exit(0);
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGSEGV, &sa, nullptr) == 0);
    ASSERT(sigaction(SIGBUS, &sa, nullptr) == 0);
    ASSERT(sigaction(SIGILL, &sa, nullptr) == 0);
    ASSERT(sigaction(SIGFPE, &sa, nullptr) == 0);

    u8* code = (u8*)mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    ASSERT(code != MAP_FAILED);
    // Some instructions to push down the ud2
    // add eax, ebx  ; 01 d8
    // movss xmm0, xmm1  ; f3 0f 10 c1
    // psadbw xmm0, xmm1  ; 66 0f f6 c1
    code[0] = 0x01;
    code[1] = 0xd8;
    code[2] = 0xf3;
    code[3] = 0x0f;
    code[4] = 0x10;
    code[5] = 0xc1;
    code[6] = 0x66;
    code[7] = 0x0f;
    code[8] = 0xf6;
    code[9] = 0xc1;
    code[10] = 0x0f;
    code[11] = 0x0b;
    expected_rip = (u64)code + 10;
    ASSERT(mprotect(code, 4096, PROT_READ | PROT_EXEC) == 0);
    ((void (*)())code)();
    return 1;
}