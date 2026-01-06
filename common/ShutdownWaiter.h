#pragma once

#if defined(__linux__)
#include <signal.h>
#endif

namespace daemon_utils {

/**
 * @brief Handles shutdown signals (SIGTERM, SIGINT) on Linux daemon builds,
 * or waits for stdin input on other platforms.
 */
class ShutdownWaiter {
   public:
    /**
     * @brief Construct a ShutdownWaiter and set up signal handling.
     *
     * On Linux daemon builds, this blocks SIGTERM and SIGINT signals.
     * On other platforms, this does nothing.
     */
    ShutdownWaiter();

    /**
     * @brief Wait for a shutdown signal or user input.
     *
     * On Linux daemon builds, this blocks until SIGTERM or SIGINT is received.
     * On other platforms, this waits for a character on stdin.
     */
    void Wait();

   private:
#if defined(__linux__)
    // Always include on linux so we don't have to leak build configuration to dependents
    sigset_t signal_set;
#endif
};

}  // namespace daemon_utils
