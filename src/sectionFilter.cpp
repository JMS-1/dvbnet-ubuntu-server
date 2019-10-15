#include "sectionFilter.hpp"

#include <linux/dvb/dmx.h>
#include <sys/ioctl.h>

bool SectionFilter::start()
{
    if (!open() || !isConnected())
    {
        return false;
    }

    dmx_sct_filter_params filter = {
        _pid,
        {0},
        0,
        DMX_IMMEDIATE_START | DMX_CHECK_CRC};

    auto pid_error = ::ioctl(_fd, DMX_SET_FILTER, &filter);

    if (pid_error != 0)
    {
        return false;
    }

    startThread();

    return true;
}
