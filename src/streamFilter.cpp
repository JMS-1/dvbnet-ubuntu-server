#include "streamFilter.hpp"

#include <linux/dvb/dmx.h>
#include <sys/ioctl.h>

bool StreamFilter::start()
{
    if (!open() || !isConnected())
    {
        return false;
    }

    auto buf_error = ::ioctl(_fd, DMX_SET_BUFFER_SIZE, 500 * 1024);

    if (buf_error != 0)
    {
        stop();

        return false;
    }

    dmx_pes_filter_params filter = {
        _pid,
        dmx_input::DMX_IN_FRONTEND,
        dmx_output::DMX_OUT_TAP,
        dmx_ts_pes::DMX_PES_OTHER,
        DMX_IMMEDIATE_START};

    auto pid_error = ::ioctl(_fd, DMX_SET_PES_FILTER, &filter);

    if (pid_error != 0)
    {
        stop();

        return false;
    }

    startThread();

    return true;
}
