#include <unistd.h>
#include "common.h"

#ifdef __x86_64__
static u64 int80(u64 nr, u64 arg1) {
    u64 ret;
    asm volatile("int 0x80" : "=a"(ret) : "a"(nr), "b"(arg1) : "memory");
    return ret;
}

int main() {
    ASSERT(int80(6, 0xffffffff) == (u64)-9);
    ASSERT(int80(6, 0x12345678ffffffff) == (u64)-9);
    ASSERT(int80(0xdeadbeef00000006, 0xffffffff) == (u64)-9);
    ASSERT(int80(0x7fff, 0) == (u64)-38);
    ASSERT(int80(0xdeadbeef00000014, 0) == (u64)getpid());
    return 0;
}
#else
int main() {
    return 0;
}
#endif
