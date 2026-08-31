#include <string.h>
#include "common.h"

#ifdef __x86_64__
int main() {
    return 0;
}
#else

struct xmmreg {
    u32 element[4];
};

struct fpstate_32 {
    u32 cw, sw, tag, ipoff, cssel, dataoff, datasel;
    u8 _st[80];
    u16 status;
    u16 magic;
    u32 _fxsr_env[6];
    u32 mxcsr;
    u32 reserved;
    u8 _fxsr_st[128];
    xmmreg _xmm[8];
};
static_assert(sizeof(fpstate_32) == 400);

struct __attribute__((packed)) sigcontext_32 {
    u16 gs, __gsh;
    u16 fs, __fsh;
    u16 es, __esh;
    u16 ds, __dsh;
    u32 di, si, bp, sp, bx, dx, cx, ax;
    u32 trapno, err, ip;
    u16 cs, __csh;
    u32 flags, sp_at_signal;
    u16 ss, __ssh;
    u32 fpstate;
    u32 oldmask;
    u32 cr2;
};

__attribute__((naked)) void store_xmms(void* out) {
    asm(R"(
        mov eax, [esp + 4]
        movups [eax + 0x00], xmm0
        movups [eax + 0x10], xmm1
        ret
    )");
}

__attribute__((naked)) void load_xmms(const void* in) {
    asm(R"(
        mov eax, [esp + 4]
        movups xmm0, [eax + 0x00]
        movups xmm1, [eax + 0x10]
        ret
    )");
}

xmmreg before[2];
xmmreg after[2];
bool handler_ran = false;

void signal_handler(int sig, sigcontext_32 sc) {
    handler_ran = true;
    ASSERT(sig == SIGUSR1);
    ASSERT(sc.fpstate != 0);
    ASSERT(sc.oldmask == (1u << (SIGURG - 1)));
    ASSERT(sc.cs == 0x23);
    fpstate_32* fp = (fpstate_32*)(unsigned long)sc.fpstate;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            ASSERT(fp->_xmm[i].element[j] == before[i].element[j]);
            fp->_xmm[i].element[j] = 0xf00d0000 + i * 4 + j;
        }
    }
    ASSERT(fp->magic == 0);
}

int main() {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            before[i].element[j] = 0xdead0000 + i * 4 + j;
        }
    }

    sigset_t blocked;
    sigemptyset(&blocked);
    sigaddset(&blocked, SIGURG);
    ASSERT(sigprocmask(SIG_BLOCK, &blocked, nullptr) == 0);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = (void (*)(int))signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);

    load_xmms(before);
    raise(SIGUSR1);
    store_xmms(after);

    ASSERT(handler_ran);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 4; j++) {
            ASSERT(after[i].element[j] == 0xf00d0000u + i * 4 + j);
        }
    }
    return 0;
}
#endif
