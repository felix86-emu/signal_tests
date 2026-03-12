#include <csignal>
#include <sys/wait.h>
#include <unistd.h>
#include "common.h"

int main(int argc, char* const* argv) {
    pid_t pid = fork();

    if (pid < 0) {
        return 1;
    }

    if (pid == 0) {
        raise(SIGURG); // default behavior is ignore
        exit(0);
    }

    // Parent process
    int status = 0;
    pid_t w = waitpid(pid, &status, 0);

    if (w == -1) {
        return 1;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return 0;
    }

    return 1;
}