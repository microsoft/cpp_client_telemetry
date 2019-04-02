#include "ECSClientSample.hpp"

#include "LogManager.hpp"

#include "auf/auf.hpp"

int main(int argc, char* argv[])
{
    auf::g_configLogHookStdoutPreinstalled = 1;
    //auf::init();

    ECSClientSample();

    //auf::stop();

    return 0;
}