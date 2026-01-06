#include "ShutdownWaiter.h"

#if defined(__linux__) && defined(MRC_DAEMON_BUILD)
#include <signal.h>
#else
#include <cstdio>
#endif
namespace daemon_utils {

ShutdownWaiter::ShutdownWaiter() {
#if defined(__linux__) && defined(MRC_DAEMON_BUILD)
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGTERM);
    sigaddset(&signal_set, SIGINT);
    sigprocmask(SIG_BLOCK, &signal_set, nullptr);
#endif
}

void ShutdownWaiter::Wait() {
#if defined(__linux__) && defined(MRC_DAEMON_BUILD)
    int sig = 0;
    sigwait(&signal_set, &sig);
#else
    (void)getchar();
#endif
}

} // namespace daemon_utils
