#include <string.h>
#include "common.h"

int main() {
    u8* base = (u8*)sbrk(0);
    ASSERT(base != (u8*)-1);

    for (int i = 0; i < 256; i++) {
        u8* chunk = (u8*)sbrk(4096);
        ASSERT(chunk != (u8*)-1);
        ASSERT(chunk == base + i * 4096);
        memset(chunk, 0x5a, 4096);
    }

    u8* big = (u8*)sbrk(16 << 20);
    ASSERT(big != (u8*)-1);
    ASSERT(big == base + 256 * 4096);
    memset(big, 0x5a, 16 << 20);

    ASSERT(sbrk(0) == base + 256 * 4096 + (16 << 20));
    return 0;
}
