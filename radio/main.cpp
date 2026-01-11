#if defined(__linux__) && defined(MRC_DAEMON_BUILD)
#include <signal.h>
#endif
#include <stdio.h>

#include <wpi/util/json.hpp>

#include "version.h"
#include "HttpClient.h"

#include <wpi/net/EventLoopRunner.hpp>
#include <wpi/net/uv/Timer.hpp>

#include "wpi/nt/NetworkTableInstance.hpp"
#include "wpi/nt/StringTopic.hpp"

struct DataStorage {
    wpi::util::Logger logger;
    wpi::nt::StringSubscriber teamSubscriber;
    wpi::nt::StringPublisher resultPublisher;

    std::unique_ptr<HttpClient> httpClient;
};

static bool startUvLoop(wpi::net::uv::Loop& loop, DataStorage& instData);
int main() {
    printf("Starting RadioDaemon\n");
    printf("\tBuild Hash: %s\n", MRC_GetGitHash());
    printf("\tBuild Timestamp: %s\n", MRC_GetBuildTimestamp());

#if defined(__linux__) && defined(MRC_DAEMON_BUILD)
    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGTERM);
    sigaddset(&signal_set, SIGINT);
    sigprocmask(SIG_BLOCK, &signal_set, nullptr);
#endif

    auto ntInst = wpi::nt::NetworkTableInstance::Create();
    ntInst.SetServer({"localhost"}, 5810);
    ntInst.StartClient("RadioDaemon");

    DataStorage instData;

    instData.teamSubscriber = ntInst.GetStringTopic("/sys/team").Subscribe("");
    instData.resultPublisher =
        ntInst.GetStringTopic("/radio/status").PublishEx("json", {{}});

    wpi::net::EventLoopRunner loopRunner;

    bool success = false;
    loopRunner.ExecSync([&success, &instData](wpi::net::uv::Loop& loop) {
        // Create HTTP client
        instData.httpClient =
            std::make_unique<HttpClient>(loop, instData.logger);

        // Connect to the completed signal
        instData.httpClient->completed.connect([&instData](
                                                   int statusCode,
                                                   std::string_view body) {
            if (statusCode >= 200 && statusCode < 300) {
                instData.resultPublisher.Set(body);
                printf("HTTP request successful, status: %d\\n", statusCode);
            } else {
                instData.resultPublisher.Set("");
                printf("HTTP request failed with status: %d\\n", statusCode);
            }
        });

        success = startUvLoop(loop, instData);
    });

    if (!success) {
        loopRunner.Stop();
        return -1;
    }

    {
#if defined(__linux__) && defined(MRC_DAEMON_BUILD)
        int sig = 0;
        sigwait(&signal_set, &sig);
#else
        (void)getchar();
#endif
    }
    loopRunner.Stop();
    ntInst.StopClient();
    wpi::nt::NetworkTableInstance::Destroy(ntInst);

    return 0;
}

static void makeHttpRequest(wpi::net::uv::Loop& loop, DataStorage& instData) {
    if (!instData.httpClient || instData.httpClient->IsBusy()) {
        return;  // Skip if no client or previous request still pending
    }

    // HTTP endpoint to poll (constant for now)
    constexpr const char* HTTP_ENDPOINT = "http://localhost:8000/status";

    instData.httpClient->Get(HTTP_ENDPOINT);
}

static bool startUvLoop(wpi::net::uv::Loop& loop, DataStorage& instData) {
    auto timer = wpi::net::uv::Timer::Create(loop);
    if (!timer) {
        return false;
    }

    // Set up timer to poll every 5 seconds
    timer->timeout.connect(
        [&loop, &instData] { makeHttpRequest(loop, instData); });

    // Start timer: 0ms initial delay, 5000ms (5 second) repeat interval
    timer->Start(wpi::net::uv::Timer::Time{0}, wpi::net::uv::Timer::Time{5000});

    return true;
}
