#include "manager.hpp"

FrontendManager::~FrontendManager()
{
    _active = false;

    for (auto &frontend : _frontends)
    {
        delete frontend.second;
    }

    _frontends.clear();
}

int makeKey(int adapter, int frontend)
{
    if (adapter < 0 || adapter > 999)
    {
        return -1;
    }

    if (frontend < 0 || frontend > 999)
    {
        return -1;
    }

    return 1000 * adapter + frontend;
}

Frontend *FrontendManager::createFrontend(int adapter, int frontend)
{
    auto key = makeKey(adapter, frontend);

    if (key < 0)
    {
        return nullptr;
    }

    if (_frontends.find(key) != _frontends.end())
    {
        return nullptr;
    }

    return _frontends[key] = new Frontend(adapter, frontend, this);
}

void FrontendManager::removeFrontend(int adapter, int frontend)
{
    if (!_active)
    {
        return;
    }

    auto key = makeKey(adapter, frontend);

    if (key < 0)
    {
        return;
    }

    if (_frontends.find(key) == _frontends.end())
    {
        return;
    }

    _frontends.erase(key);
}