#ifndef _DVBNET_FRONTEND_H
#define _DVBNET_FRONTEND_H 1

#include <linux/dvb/frontend.h>

#include <map>

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
    std::map<__u16, Filter *> _filters;
    int _fd;

public:
    const int adapter;
    const int frontend;

public:
    const bool isOpen() { return _fd >= 0; }

public:
    bool open();
    const fe_status getStatus();
    bool tune();
    void close();

public:
    void createSectionFilter(__u16 pid);
    void createStreamFilter(__u16 pid);
    bool startFilter(__u16 pid);
    bool removeFilter(__u16 pid);
    void removeAllFilters();
};

#endif