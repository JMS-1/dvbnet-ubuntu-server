#ifndef _DVBNET_FRONTEND_H
#define _DVBNET_FRONTEND_H 1

#include <linux/dvb/frontend.h>

#include "filter.hpp"

class Frontend;

class Frontend
{
public:
    Frontend(int adapter, int frontend) : adapter(adapter), frontend(frontend), _fd(-1)
    {
    }

    ~Frontend()
    {
        close();
    }

private:
    int _fd;

public:
    const int adapter;
    const int frontend;

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