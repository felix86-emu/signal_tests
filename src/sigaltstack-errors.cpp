#include <string.h>
#include "common.h"

constexpr size_t ALTSZ = 1024 * 1024;

int main() {
    void* alt = malloc(ALTSZ);

    stack_t small;
    small.ss_sp = alt;
    small.ss_size = 1;
    small.ss_flags = 0;
    errno = 0;
    ASSERT(sigaltstack(&small, nullptr) == -1);
    ASSERT(errno == ENOMEM);

    stack_t bad_flags;
    bad_flags.ss_sp = alt;
    bad_flags.ss_size = ALTSZ;
    bad_flags.ss_flags = 0x1234;
    errno = 0;
    ASSERT(sigaltstack(&bad_flags, nullptr) == -1);
    ASSERT(errno == EINVAL);

    stack_t q;
    ASSERT(sigaltstack(nullptr, &q) == 0);
    ASSERT(q.ss_sp == nullptr);
    ASSERT(q.ss_size == 0);
    ASSERT(q.ss_flags == SS_DISABLE);

    stack_t good;
    good.ss_sp = alt;
    good.ss_size = ALTSZ;
    good.ss_flags = 0;
    ASSERT(sigaltstack(&good, nullptr) == 0);
    errno = 0;
    ASSERT(sigaltstack(&small, nullptr) == -1);
    ASSERT(errno == ENOMEM);
    ASSERT(sigaltstack(nullptr, &q) == 0);
    ASSERT(q.ss_sp == alt);
    ASSERT(q.ss_size == ALTSZ);
    ASSERT(q.ss_flags == 0);
    return 0;
}
