#include "frontend.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>

bool Frontend::close()
{
    if (_fd < 0)
    {
        return false;
    }

    ::close(_fd);

    _fd = -1;

    return true;
}

bool Frontend::open()
{
    close();

    _fd = ::open("/dev/dvb/adapter0/frontend0", O_RDWR);

    return (_fd >= 0);
}

const fe_status Frontend::getStatus()
{
    if (!isOpen())
    {
        throw "frontend not open";
    }

    fe_status status;

    if (::ioctl(_fd, FE_READ_STATUS, &status) < 0)
    {
        throw "unable to access frontend";
    }

    return status;
}
