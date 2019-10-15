#include "filter.hpp"
#include "frontend.hpp"

#include <fcntl.h>
#include <unistd.h>

void Filter::feeder()
{
    auto dump = ::open(_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

    ssize_t total = 0;

    auto buffer = ::malloc(_bufsize);

    while (total < _limit)
    {
        auto bytes = ::read(_fd, buffer, _bufsize);

        if (bytes <= 0)
        {
            return;
        }

        ::write(dump, buffer, bytes);

        total += bytes;
    }

    ::close(dump);

    ::printf("total %ld\n", total);
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

    thread->join();
}

bool Filter::open()
{
    stop();

    char path[40];

    ::sprintf(path, "/dev/dvb/adapter%i/demux%i", _frontend.adapter, _frontend.frontend);

    _fd = ::open(path, O_RDWR);

    return _fd >= 0;
}

std::thread *Filter::startThread()
{
    if (_thread)
    {
        return _thread;
    }

    _thread = new std::thread(&Filter::feeder, this);

    return _thread;
}
