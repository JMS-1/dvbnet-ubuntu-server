#include "frontend.hpp"

#include <sys/ioctl.h>

class SectionFilter : public Filter
{
    friend class Frontend;

private:
    SectionFilter(Frontend &frontend, __u16 pid) : Filter(frontend, pid) {}

public:
    bool start()
    {
        if (!open())
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

        startThread("dump.epg", 1000, 200000)->join();

        return true;
    }
};
