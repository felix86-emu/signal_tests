#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include "common.h"

typedef int (*func_t)();

static void write_code(u8* where, int value) {
    const u8 code[] = {0xB8, 0x00, 0x00, 0x00, 0x00, 0xC3};
    memcpy(where, code, sizeof(code));
    memcpy(where + 1, &value, sizeof(value));
}

static int make_segment(int value) {
    int id = shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0600);
    ASSERT(id != -1);
    u8* temp = (u8*)shmat(id, nullptr, 0);
    ASSERT(temp != (u8*)-1);
    write_code(temp, value);
    ASSERT(shmdt(temp) == 0);
    return id;
}

int main() {
    int first = make_segment(0xab01);
    int second = make_segment(0xab02);

    u8* page = (u8*)mmap(nullptr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ASSERT(page != MAP_FAILED);
    write_code(page, 0xaaaa);
    func_t func = (func_t)page;
    ASSERT(func() == 0xaaaa);

    ASSERT(shmat(first, page, SHM_REMAP) == page);
    ASSERT(mprotect(page, 4096, PROT_READ | PROT_EXEC) == 0);
    int first_result = func();

    ASSERT(shmdt(page) == 0);
    ASSERT(shmat(second, page, 0) == page);
    ASSERT(mprotect(page, 4096, PROT_READ | PROT_EXEC) == 0);
    int second_result = func();

    ASSERT(shmdt(page) == 0);
    shmctl(first, IPC_RMID, nullptr);
    shmctl(second, IPC_RMID, nullptr);

    ASSERT(first_result == 0xab01);
    ASSERT(second_result == 0xab02);
    return 0;
}
