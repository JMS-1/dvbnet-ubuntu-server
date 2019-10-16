#ifndef _DVBNET_FRONTEND_H
#define _DVBNET_FRONTEND_H 1

#include <unistd.h>

#include <map>
#include <thread>

#include "messages.hpp"

class Filter;

class FrontendManager;

class Frontend
{
    friend class Filter;
    friend class FrontendManager;

public:
    Frontend(int tcp, FrontendManager *_manager);
    ~Frontend();

private:
    bool _active;
    FrontendManager *_manager;
    int _fd;
    int _tcp;
    int adapter;
    int frontend;
    std::map<__u16, Filter *> _filters;
    std::thread *_listener;
    std::thread *_status;

private:
    void readStatus();
    void waitRequest();
    bool processConnect();
    bool processTune();
    bool processAddSection();
    bool processAddStream();
    bool processRemoveFilter();
    bool processRemoveAllFilters();
    void close(bool nowait);
    void removeFilter(__u16 pid);
    void removeAllFilters();
    void close();

private:
    void sendResponse(response *data, int payloadSize)
    {
        data->len = payloadSize;

        ::write(_tcp, data, sizeof(response) + payloadSize);
    }
};

#endif