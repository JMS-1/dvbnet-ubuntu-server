#ifndef _DVBNET_THREADTOOLS_H
#define _DVBNET_THREADTOOLS_H 1

#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#include <thread>

class ThreadTools
{
private:
    static void ignore_signal(int signo)
    {
#ifdef DEBUG
        ::printf("caught %d\n", signo);
#endif
        signal();
    }

public:
    static void signal()
    {
        ::signal(SIGUSR1, ignore_signal);
    }

    static void join(std::thread *&thread, bool nowait = false)
    {
        const auto t = thread;

        if (!t)
        {
            return;
        }

        thread = nullptr;

        if (nowait)
        {
            return;
        }

        try
        {
            ::pthread_kill(t->native_handle(), SIGUSR1);

            t->join();
        }
        catch (...)
        {
            ::printf("unable to terminate thread property\n");
        }
    }
};

#endif