#include "common.h"
#include <string.h>
#include <sys/mman.h>

#ifndef __x86_64__
int main() {
    return 0;
}
#else

static const u8 code_bytes[] = {
    0x48, 0x89, 0xfc,
    0xb8, 0x0f, 0x00, 0x00, 0x00,
    0x0f, 0x05,
    0x40,
    0x89, 0xc3,
    0xb8, 0x01, 0x00, 0x00, 0x00,
    0xcd, 0x80,
};
static constexpr u64 after_sigreturn_offset = 10;

int main() {
    u8* low = (u8*)mmap(nullptr, 0x3000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    ASSERT(low != MAP_FAILED);
    ASSERT((u64)low + 0x3000 <= 0x100000000ull);

    u8* code = low;
    memcpy(code, code_bytes, sizeof(code_bytes));

    u8* stack32_top = low + 0x2000;
    ucontext_t* frame = (ucontext_t*)(low + 0x2000);
    memset(frame, 0, sizeof(*frame));

    greg_t* gregs = frame->uc_mcontext.gregs;
    gregs[REG_RAX] = 0xFFFFFFFF;
    gregs[REG_RSP] = (u64)stack32_top - 16;
    gregs[REG_RIP] = (u64)code + after_sigreturn_offset;
    gregs[REG_EFL] = 0x202;
    gregs[REG_CSGSFS] = 0x23 | (0x2bull << 48);

    ((void (*)(void*))code)(frame);
    return 1;
}

#endif
