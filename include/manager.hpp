#ifndef _DVBNET_MANAGER_H
#define _DVBNET_MANAGER_H 1

#include <map>

#include "frontend.hpp"

class FrontendManager
{
    friend class Frontend;

public:
    FrontendManager() : _active(true) {}
    ~FrontendManager();

private:
    std::map<int, Frontend *> _frontends;
    bool _active;

public:
    Frontend *createFrontend(int adapter, int frontend);

private:
    void removeFrontend(int adapter, int frontend);
};

#endif