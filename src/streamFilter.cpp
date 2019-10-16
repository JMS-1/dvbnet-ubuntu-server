#include "streamFilter.hpp"

#include <linux/dvb/dmx.h>
#include <sys/ioctl.h>

/*
    Aktiviert die Entgegennahme eines Nutzdatenstroms.
*/
bool StreamFilter::start()
{
    // Dateizugriff zum Demultiplexer öffnen.
    auto fd = open();

    if (fd < 0)
    {
        return false;
    }

    // Vergrößerten Zwischenspeicher anlegen.
    ::ioctl(fd, DMX_SET_BUFFER_SIZE, 500 * 1024) != 0;

    // Datenstrom beim Demultiplexer anmelden.
    dmx_pes_filter_params filter = {
        _pid,
        dmx_input::DMX_IN_FRONTEND,
        dmx_output::DMX_OUT_TAP,
        dmx_ts_pes::DMX_PES_OTHER,
        DMX_IMMEDIATE_START | DMX_CHECK_CRC};

    if (::ioctl(fd, DMX_SET_PES_FILTER, &filter) != 0)
    {
        // Entgegennahme der Daten nicht möglich.
        stop();

        return false;
    }

    // Entgegennahme der Daten und Weitergabe aktivieren.
    startThread();

    return true;
}
