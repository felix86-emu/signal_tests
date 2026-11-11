#include <string.h>
#include <sys/mman.h>
#include "common.h"

u8* page;

void signal_handler(int sig, siginfo_t* info, void* ctx) {
    ASSERT(sig == SIGSEGV);
    ASSERT(info->si_code == SEGV_ACCERR);
    ASSERT(info->si_addr == page);
    exit(0);
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = signal_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    ASSERT(sigaction(SIGSEGV, &sa, nullptr) == 0);

    page = (u8*)mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ASSERT(page != MAP_FAILED);
    const u8 code[] = {0xB8, 0x2A, 0x00, 0x00, 0x00, 0xC3};
    memcpy(page, code, sizeof(code));
    ASSERT(mprotect(page, 4096, PROT_READ | PROT_EXEC) == 0);

    int (*func)() = (int (*)())page;
    ASSERT(func() == 42);

    ASSERT(mprotect(page, 4096, PROT_READ) == 0);
    func();
    ASSERT(false);
    return 1;
}
