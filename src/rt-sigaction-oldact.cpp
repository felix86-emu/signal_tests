#include <string.h>
#include <syscall.h>
#include "common.h"

#ifndef SA_RESTORER
#define SA_RESTORER 0x04000000
#endif

struct kernel_sigaction {
    void* handler;
    unsigned long flags;
    void* restorer;
    unsigned long mask[sizeof(void*) == 8 ? 1 : 2];
};

void dummy_handler(int) {}
extern "C" void dummy_restorer();
__attribute__((naked)) void dummy_restorer_impl() {
    asm("ret");
}

int main() {
    kernel_sigaction act;
    memset(&act, 0, sizeof(act));
    act.handler = (void*)dummy_handler;
    act.flags = SA_RESTORER | SA_NODEFER;
    act.restorer = (void*)dummy_restorer_impl;
    act.mask[0] = 1ul << (SIGURG - 1);
    ASSERT(syscall(SYS_rt_sigaction, SIGPWR, &act, nullptr, 8) == 0);

    kernel_sigaction old;
    memset(&old, 0xAA, sizeof(old));
    ASSERT(syscall(SYS_rt_sigaction, SIGPWR, nullptr, &old, 8) == 0);
    ASSERT(old.handler == (void*)dummy_handler);
    ASSERT(old.flags == (unsigned long)(SA_RESTORER | SA_NODEFER));
    ASSERT(old.mask[0] == 1ul << (SIGURG - 1));
    ASSERT(old.restorer == (void*)dummy_restorer_impl);
    return 0;
}
