#include "manager.hpp"

#include <sys/socket.h>
#include <unistd.h>

void FrontendManager::listener()
{
    for (;;)
    {
        sockaddr_in client = {0};
        socklen_t len = sizeof(client);

        auto fd = ::accept(_fd, reinterpret_cast<sockaddr *>(&client), &len);

        if (fd < 0)
        {
            break;
        }

        new Frontend(fd, this);
    }
}

bool FrontendManager::listen(in_port_t port /* = 29713 */)
{
    close();

    _fd = ::socket(AF_INET, SOCK_STREAM, 0);

    if (_fd < 0)
    {
        return false;
    }

    sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = port,
        .sin_addr = {.s_addr = INADDR_ANY},
    };

    if (::bind(_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0)
    {
        close();

        return false;
    }

    if (::listen(_fd, 5) != 0)
    {
        close();

        return false;
    }

    _listen = new std::thread(&FrontendManager::listener, this);
}

void FrontendManager::close()
{
    auto fd = _fd;

    if (fd < 0)
    {
        return;
    }

    _fd = -1;

    ::close(fd);

    auto thread = _listen;

    if (!thread)
    {
        return;
    }

    _listen = nullptr;

    if (thread->joinable())
    {
        thread->join();
    }
}

FrontendManager::~FrontendManager()
{
    _active = false;

    for (auto &frontend : _frontends)
    {
        delete frontend.second;
    }

    _frontends.clear();

    close();
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

bool FrontendManager::addFrontend(Frontend *frontend)
{
    if (!_active)
    {
        return false;
    }

    auto key = makeKey(frontend->adapter, frontend->frontend);

    if (key < 0)
    {
        return false;
    }

    if (_frontends.find(key) != _frontends.end())
    {
        return false;
    }

    _frontends[key] = frontend;

    return true;
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