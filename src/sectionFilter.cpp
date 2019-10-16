#include "sectionFilter.hpp"

#include <linux/dvb/dmx.h>
#include <sys/ioctl.h>

/*
    Beginnt mit dem Empfang der Daten für einen Kontrolldatenstrom.
*/
bool SectionFilter::start()
{
    // Zugang zum Demultiplexer anlegen.
    auto fd = open();

    if (fd < 0)
    {
        return false;
    }

    // Datenstrom beim Demultiplexer anmelden.
    dmx_sct_filter_params filter = {_pid, {0}, 0, DMX_IMMEDIATE_START | DMX_CHECK_CRC};

    if (::ioctl(fd, DMX_SET_FILTER, &filter) != 0)
    {
        // Datenannahme nicht möglich.
        stop();

        return false;
    }

    // Entgegennahme der Daten aktivieren.
    startThread();

    return true;
}
