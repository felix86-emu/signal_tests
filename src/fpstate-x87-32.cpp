#include <string.h>
#include "common.h"

#ifdef __x86_64__
int main() {
    return 0;
}
#else

struct fpstate_32 {
    u32 cw, sw, tag, ipoff, cssel, dataoff, datasel;
    u8 _st[80];
    u16 status, magic;
    u32 _fxsr_env[6];
    u32 mxcsr, reserved;
    u8 _fxsr_st[128];
    u8 _xmm[128];
};
static_assert(sizeof(fpstate_32) == 400);

__attribute__((naked)) void set_cw(u32 v) {
    asm(R"(
        mov eax, [esp + 4]
        mov [esp - 8], eax
        fldcw [esp - 8]
        ret
    )");
}

__attribute__((naked)) u32 get_cw() {
    asm(R"(
        mov dword ptr [esp - 8], 0
        fnstcw [esp - 8]
        mov eax, [esp - 8]
        ret
    )");
}

__attribute__((naked)) void load_st0(void* p) {
    asm(R"(
        finit
        mov eax, [esp + 4]
        fld tbyte ptr [eax]
        ret
    )");
}

__attribute__((naked)) void store_st0(void* p) {
    asm(R"(
        mov eax, [esp + 4]
        fstp tbyte ptr [eax]
        ret
    )");
}

u8 st0_in[10] = {0, 0, 0, 0, 0, 0, 0, 0xC0, 0xFF, 0x3F};
u8 st0_new[10] = {0, 0, 0, 0, 0, 0, 0, 0xA0, 0x00, 0x40};
u8 st0_out[10];
bool handler_ran = false;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ucontext_t* uc = (ucontext_t*)ctx;
    fpstate_32* fp = (fpstate_32*)uc->uc_mcontext.fpregs;
    handler_ran = true;

    ASSERT((fp->cw >> 16) == 0xffff);
    ASSERT((fp->sw >> 16) == 0xffff);
    ASSERT((fp->tag >> 16) == 0xffff);
    ASSERT((fp->cw & 0xffff) == 0x037f);
    ASSERT(memcmp(fp->_st, st0_in, 10) == 0);

    fp->cw = 0xffff027f;
    memcpy(fp->_st, st0_new, 10);
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);

    set_cw(0x037f);
    load_st0(st0_in);
    raise(SIGUSR1);
    ASSERT(handler_ran);
    ASSERT(get_cw() == 0x027f);
    store_st0(st0_out);
    ASSERT(memcmp(st0_out, st0_new, 10) == 0);
    return 0;
}
#endif
