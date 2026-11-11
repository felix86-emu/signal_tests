#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include "common.h"

typedef int (*func_t)();

static void write_code(u8* where, int value) {
    const u8 code[] = {0xB8, 0x00, 0x00, 0x00, 0x00, 0xC3};
    memcpy(where, code, sizeof(code));
    memcpy(where + 1, &value, sizeof(value));
}

int main() {
    u8* base = (u8*)mmap(nullptr, 3 * 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ASSERT(base != MAP_FAILED);

    func_t first = (func_t)base;

    write_code(base, 1);
    ASSERT(first() == 1);

    ASSERT(munmap(base + 4096, 4096) == 0);
    ASSERT(mprotect(base, 3 * 4096, PROT_READ | PROT_WRITE | PROT_EXEC) == -1);
    ASSERT(errno == ENOMEM);


    write_code(base, 2);
    ASSERT(first() == 2);
    return 0;
}
