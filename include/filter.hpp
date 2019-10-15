#ifndef _DVBNET_FILTER_H
#define _DVBNET_FILTER_H 1

#include <thread>

#include <linux/types.h>

class Frontend;

class Filter
{
    friend class Frontend;

protected:
    Filter(Frontend &frontend, __u16 pid, int bufsize, const char *path)
        : _bufsize(bufsize),
          _connected(true),
          _fd(-1),
          _path(path),
          _pid(pid),
          _thread(nullptr),
          _frontend(frontend)
    {
    }

protected:
    virtual ~Filter()
    {
        _connected = false;

        stop();
    }

private:
    bool _connected;
    Frontend &_frontend;
    std::thread *_thread;
    const char *_path;
    const int _bufsize;

private:
    void feeder();

protected:
    const __u16 _pid;
    int _fd;

protected:
    bool open();
    void startThread();
    const bool isConnected() { return _connected; }

public:
    virtual bool start() = 0;
    virtual void stop();
};

#endif