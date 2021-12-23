#ifndef _DVBNET_THREADTOOLS_H
#define _DVBNET_THREADTOOLS_H 1

#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#include <thread>

class ThreadTools
{
public:
    static void join(std::thread *&thread, bool nowait = false)
    {
        const auto t = thread;

        if (!t)
            return;

        thread = nullptr;

        if (nowait)
            return;

        try
        {
            t->join();
        }
        catch (...)
        {
            ::printf("unable to terminate thread property\n");
        }
    }
};

#endif