#include "filter.hpp"

#include <linux/dvb/dmx.h>
#include <sys/ioctl.h>

class StreamFilter : public Filter
{
    friend class Frontend;

private:
    StreamFilter(Frontend &frontend, __u16 pid) : Filter(frontend, pid, "dump.vid", 10000, 2000000) {}

public:
    bool start()
    {
        if (!open())
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

        startThread()->join();

        return true;
    }
};