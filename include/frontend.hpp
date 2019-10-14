#ifndef _DVBNET_FRONTEND_H
#define _DVBNET_FRONTEND_H 1

#include <thread>

#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>

class Frontend;

class Filter
{
protected:
    Filter(Frontend &frontend, __u16 pid) : _frontend(frontend), _connected(true), _pid(pid), _fd(-1), _thread(nullptr) {}

public:
    virtual ~Filter()
    {
        _connected = false;

        stop();
    }

private:
    bool _connected;
    Frontend &_frontend;
    std::thread *_thread;

protected:
    const __u16 _pid;
    int _fd;

protected:
    std::thread *startThread(const char *path, int bufsize, int limit);
    bool open();

public:
    virtual bool start() = 0;
    virtual void stop();
};

class Frontend
{
public:
    Frontend(int adapter, int frontend) : _adapter(adapter), _frontend(frontend), _fd(-1)
    {
    }

    ~Frontend()
    {
        close();
    }

private:
    const int _adapter;
    const int _frontend;
    int _fd;

public:
    const bool isOpen() { return _fd >= 0; }

public:
    bool close();
    bool open();
    const fe_status getStatus();
    Filter &createSectionFilter(__u16 pid);
    Filter &createStreamFilter(__u16 pid);
    int tune();
};

#endif