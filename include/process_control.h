#pragma once
#include <csignal>  // for kill()
#include <string>

namespace ProcessControl {
    bool KillProcess(int pid);
    bool SuspendProcess(int pid);
    bool ResumeProcess(int pid);
}
