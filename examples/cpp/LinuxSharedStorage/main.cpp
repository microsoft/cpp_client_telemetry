#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>

// This example is designed to run on Linux only
#include <sys/types.h>
#include <unistd.h>

#include "LogManager.hpp"

using namespace MAT;

const char*  TOKEN = "xxx";
const size_t MAX_CLIENTS       = 10;    // Max number of clients to spawn
const size_t MAX_DURATION      = 10;    // Server test duration in sec
const char*  CACHE_PATH        = "storage.db";

// Define it once per .exe or .dll in any compilation module
LOGMANAGER_INSTANCE

void initServer()
{
    // LogManager configuration
    auto& config = LogManager::GetLogConfiguration();

    // LogManager initialization
    printf("[S] Initialize\n");
    ILogger *logger = LogManager::Initialize(TOKEN);

    printf("[S] Processing client events");
    for(size_t i=0; i<MAX_DURATION; i++)
    {
        printf(".");
        fflush(stdout);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    printf("\n");

    printf("[S] FlushAndTeardown...\n");

    LogManager::FlushAndTeardown();
    printf("[S] [ DONE ]\n");
}

void initClient()
{
    // LogManager configuration
    auto& config = LogManager::GetLogConfiguration();

    // LogManager initialization
    printf("[C] Initialize\n");
    auto pid = getpid();

    ILogger *logger = LogManager::Initialize(TOKEN);
    LogManager::PauseTransmission();

    printf("[C] Send My.Simple.Event for pid=%ld\n", (long)pid);
    EventProperties evt("My.Simple.Event",
    {
        { "clientPid", long(pid) }
    });
    logger->LogEvent(evt);

    printf("[C] FlushAndTeardown\n");
    LogManager::FlushAndTeardown();
}

void printUsage()
{
    printf("Usage: SharedStorage [-c|-s|-t]\n");
    printf("options:                       \n");
    printf("   -c   - start client         \n");
    printf("   -s   - start server         \n");
    printf("   -t   - test client/server   \n");
    printf("                               \n");
}

int main(int argc, char *argv[])
{
    if (argc==1)
    {
        printUsage();
        return 0;
    }

    std::string arg1 = argv[1];
    if (arg1=="-c")
    {
        initClient();
        return 0;
    } else
    if (arg1=="-s")
    {
        initServer();
        return 0;
    } else
    if (arg1=="-t")
    {
        // Start 1 server
        std::string srvCmd = "sh -c \"";
        srvCmd+= argv[0];
        srvCmd+=" -s ";
        srvCmd+="</dev/null &\"";
        system(srvCmd.c_str());

        // Start a few clients
        std::string cliCmd ="sh -c \"";
        cliCmd+= argv[0];
        cliCmd+=" -c ";
        cliCmd+="</dev/null >/dev/null &\"";
        for(size_t i=0; i<MAX_CLIENTS; i++)
        {
            system(cliCmd.c_str());
        }

        // Don't return to command line until the server is done running
        std::this_thread::sleep_for(std::chrono::seconds(MAX_DURATION+1));
        return 0;
    }
    printUsage();
    return 0;
}
