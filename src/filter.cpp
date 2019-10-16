#include "filter.hpp"
#include "frontend.hpp"

#include <fcntl.h>
#include <unistd.h>

void Filter::feeder()
{
    auto bufsize = _type == frontend_response::section ? 1000 : 250000;

    response *data = reinterpret_cast<response *>(::malloc(sizeof(response) + bufsize));

    data->type = _type;
    data->pid = _pid;

    auto buffer = data->payload;

    for (;;)
    {
        auto bytes = ::read(_fd, buffer, bufsize);

        if (bytes <= 0)
        {
            break;
        }

        data->len = bytes;

        _frontend.sendResponse(data, bytes);
    }

    ::free(data);
}

void Filter::stop()
{
    const auto fd = _fd;

    if (fd < 0)
    {
        return;
    }

    _fd = -1;

    close(fd);

    const auto thread = _thread;

    if (!thread)
    {
        return;
    }

    _thread = nullptr;

    if (thread->joinable())
    {
        thread->join();
    }
}

bool Filter::open()
{
    stop();

    char path[40];

    ::sprintf(path, "/dev/dvb/adapter%i/demux%i", _frontend.adapter, _frontend.frontend);

    _fd = ::open(path, O_RDWR);

    return _fd >= 0;
}

void Filter::startThread()
{
    if (_thread)
    {
        return;
    }

    _thread = new std::thread(&Filter::feeder, this);
}
