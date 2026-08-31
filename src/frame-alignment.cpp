#include <string.h>
#include "common.h"

u64 entry_sp = 0;
int count = 0;

extern "C" __attribute__((noinline)) void record(u64 sp) {
    entry_sp = sp;
    count++;
}

__attribute__((naked)) void signal_handler(int sig, siginfo_t* info, void* ctx) {
#ifdef __x86_64__
    asm(R"(
        mov rdi, rsp
        mov rbp, rsp
        and rsp, -16
        call record
        mov rsp, rbp
        ret
    )");
#else
    asm(R"(
        mov ecx, esp
        mov ebp, esp
        and esp, -16
        sub esp, 12
        push ecx
        call record
        mov esp, ebp
        ret
    )");
#endif
}

int main() {
    void* alt = malloc(1024 * 1024);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = (void (*)(int, siginfo_t*, void*))signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);

    raise(SIGUSR1);
    ASSERT(count == 1);
    ASSERT(entry_sp % 16 == 16 - sizeof(void*));

    stack_t ss;
    ss.ss_sp = (u8*)alt + 7;
    ss.ss_size = 1024 * 1024 - 7;
    ss.ss_flags = 0;
    ASSERT(sigaltstack(&ss, nullptr) == 0);
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    ASSERT(sigaction(SIGUSR1, &sa, nullptr) == 0);
    raise(SIGUSR1);
    ASSERT(count == 2);
    ASSERT(entry_sp % 16 == 16 - sizeof(void*));
    struct sigaction sa2;
    memset(&sa2, 0, sizeof(sa2));
    sa2.sa_handler = (void (*)(int))signal_handler;
    sa2.sa_flags = 0;
    sigemptyset(&sa2.sa_mask);
    ASSERT(sigaction(SIGUSR2, &sa2, nullptr) == 0);
    raise(SIGUSR2);
    ASSERT(count == 3);
    ASSERT(entry_sp % 16 == 16 - sizeof(void*));
    return 0;
}
