#include "frontend.hpp"

#include <fcntl.h>
#include <unistd.h>

void streamToFile(int fd, const char *target, int bufsize, int limit)
{
    auto dump = ::open(target, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

    ssize_t total = 0;

    auto buffer = ::malloc(bufsize);

    while (total < limit)
    {
        auto bytes = ::read(fd, buffer, bufsize);

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

    _fd = ::open("/dev/dvb/adapter0/demux0", O_RDWR);

    return _fd >= 0;
}

std::thread *Filter::startThread(const char *path, int bufsize, int limit)
{
    if (_thread)
    {
        return _thread;
    }

    _thread = new std::thread(streamToFile, _fd, path, bufsize, limit);

    return _thread;
}
