#include "process_control.h"
#include <signal.h>
#include <unistd.h>

namespace ProcessControl {

    bool KillProcess(int pid) {
        return kill(pid, SIGKILL) == 0;
    }

    bool SuspendProcess(int pid) {
        return kill(pid, SIGSTOP) == 0;
    }

    bool ResumeProcess(int pid) {
        return kill(pid, SIGCONT) == 0;
    }
}
