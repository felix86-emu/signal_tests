#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ucontext.h>
#include <unistd.h>

#define ASSERT(cond)                                                                                                                                 \
    do {                                                                                                                                             \
        if (!(cond)) {                                                                                                                               \
            printf("Condition %s failed!\n", #cond);                                                                                                 \
            exit(1);                                                                                                                                 \
        }                                                                                                                                            \
    } while (0)

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

#if !defined(REG_RIP) && defined(REG_EIP)
#define REG_RIP REG_EIP
#endif

#if !defined(REG_RAX) && defined(REG_EAX)
#define REG_RAX REG_EAX
#endif