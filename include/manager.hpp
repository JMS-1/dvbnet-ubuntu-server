#ifndef _DVBNET_MANAGER_H
#define _DVBNET_MANAGER_H 1

#include <netinet/in.h>

#include <map>
#include <thread>

#include "frontend.hpp"

class FrontendManager
{
    friend class Frontend;

public:
    FrontendManager() : _active(true), _fd(-1), _listen(nullptr) {}
    ~FrontendManager();

private:
    void close();

private:
    std::map<int, Frontend *> _frontends;
    std::thread *_listen;
    bool _active;
    int _fd;

public:
    bool addFrontend(Frontend *frontend);
    bool listen(in_port_t = 29713);

private:
    void removeFrontend(int adapter, int frontend);
    void listener();
};

#endif