#include <csignal>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>
#include "common.h"

enum Behavior { Ignore, Stop, Core, Terminate, What };

Behavior get_behavior(int sig) {
    switch (sig) {
    case SIGHUP:
    case SIGALRM:
    case SIGINT:
    case SIGIO:
    case SIGPIPE:
    case SIGPROF:
    case SIGPWR:
    case SIGSTKFLT:
    case SIGTERM:
    case SIGUSR1:
    case SIGUSR2:
    case SIGVTALRM:
    case 32 ... 64: /* Realtime signals */ {
        return Terminate;
    }
    case SIGABRT:
    case SIGBUS:
    case SIGFPE:
    case SIGILL:
    case SIGQUIT:
    case SIGSEGV:
    case SIGSYS:
    case SIGTRAP:
    case SIGXCPU:
    case SIGXFSZ: {
        return Core;
    }
    case SIGCONT:
    case SIGURG:
    case SIGWINCH:
    case SIGCHLD: {
        return Ignore;
    }
    case SIGTSTP:
    case SIGTTIN:
    case SIGTTOU:
    case SIGSTOP: {
        return Stop;
    }
    default: {
        return What;
    }
    }
}

int main(int argc, char* const* argv) {
    for (int i = 1; i <= 64; i++) {
        if (i == SIGKILL) {
            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            return 1;
        }

        if (pid == 0) {
            if (i != SIGSTOP) {
                struct sigaction sa;
                sa.sa_flags = 0;
                sigemptyset(&sa.sa_mask);
                sa.sa_sigaction = (decltype(sa.sa_sigaction))SIG_DFL;
                int r = syscall(SYS_rt_sigaction, i, &sa, nullptr, sizeof(u64), 0, 0);
                if (r != 0) {
                    return 1;
                }
            }
            ASSERT(syscall(SYS_tgkill, getpid(), gettid(), i) == 0);
            return 42;
        }

        int status = 0;
        pid_t w = waitpid(pid, &status, WUNTRACED);
        if (w == -1) {
            return 1;
        }

        Behavior beh = get_behavior(i);
        switch (beh) {
        case Terminate:
        case Core: {
            if (WIFSIGNALED(status) && WTERMSIG(status) == i) {
                break;
            } else {
                return 1;
            }
        }
        case Stop: {
            if (WIFSTOPPED(status) && WSTOPSIG(status) == i) {
                kill(pid, SIGCONT);
                pid_t w = waitpid(pid, &status, 0);
                if (w == -1) {
                    return 1;
                }

                if (WIFEXITED(status) && (WEXITSTATUS(status) & 0xFF) == 42) {
                    break;
                } else {
                    return 1;
                }
            } else {
                return 1;
            }
        }
        case Ignore: {
            if (WIFEXITED(status) && (WEXITSTATUS(status) & 0xFF) == 42) {
                break;
            } else {
                return 1;
            }
        }
        case What: {
            return 1;
        }
        }
    }
    return 0;
}