#ifndef _DVBNET_FILTER_H
#define _DVBNET_FILTER_H 1

#include <thread>

#include <linux/types.h>

class Frontend;

class Filter
{
protected:
    Filter(Frontend &frontend, __u16 pid, const char *path, int bufsize, int limit)
        : _bufsize(bufsize),
          _connected(true),
          _fd(-1),
          _limit(limit),
          _path(path),
          _pid(pid),
          _thread(nullptr),
          _frontend(frontend)
    {
    }

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
    const char *_path;
    const int _bufsize;
    const int _limit;

private:
    void feeder();

protected:
    const __u16 _pid;
    int _fd;

protected:
    std::thread *startThread();
    bool open();

public:
    virtual bool start() = 0;
    virtual void stop();
};

#endif