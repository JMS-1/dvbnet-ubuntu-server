#include "filter.hpp"

#include <fcntl.h>
#include <string.h>
#include <linux/dvb/dmx.h>
#include <sys/ioctl.h>

#include "frontend.hpp"
#include "threadTools.hpp"

#define TSPACKETSIZE 188
#define TSMAGICBYTE 0x47

/*
    Liest einen Datenstrom (einzelne PID) vom Demultiplexer und meldet
    das Ergebnis an die Frontendinstanz zur Weitergabe an den Client.
*/
void Filter::feeder()
{
    ThreadTools::signal();

#ifdef DEBUG
    // Protokollausgabe.
    ::printf("+feeder\n");
#endif

    // Die Größe des Zwischenspeichers orientiert sich an der Art des Datenstroms.
    const auto bufsize = 100 * TSPACKETSIZE;

    // Zusätzlich zu den Nutzdaten muss auch immer die Kontrollstruktur aufgesetzt werden.
    auto payload = reinterpret_cast<u_char *>(::malloc(2 * (bufsize + TSPACKETSIZE)));

    // Nutzdatenbereich ermitteln.
    auto buffer = payload + bufsize + TSPACKETSIZE;
    auto prev = 0;

    ssize_t skipped = 0, total = 0;
    auto overflow = 0;

    for (;;)
    {
        // Daten aus dem Demultiplexer auslesen.
        auto bytes = read(_fd, buffer + prev, bufsize);

        // Sobald keine Daten mehr ankommen wird das Auslesen beendet.
        if (bytes < 0)
        {
            if (errno != EOVERFLOW)
                break;

            overflow++;

            continue;
        }

        // Statistik.
        total += bytes;

        // Aktueller Analysebereich.
        bytes += prev;

        // Nutzdatenbereich füllen.
        auto source = buffer, end = buffer + bytes, dest = payload;

        while (source < end)
            if (*source != TSMAGICBYTE)
            {
                // Wir haben noch nicht den Anfang eines TS Paketes erreicht.
                skipped++;
                source++;
            }
            else if (source + TSPACKETSIZE >= end)
            {
                // Das TS Paket ist noch unvollständig.
                break;
            }
            else if (source[TSPACKETSIZE] != TSMAGICBYTE)
            {
                // Es ist kein TS Paket.
                skipped++;
                source++;
            }
            else
            {
                // PID ermitteln.
                auto pidHigh = static_cast<__u16>(source[1]);
                auto pidLow = static_cast<__u16>(source[2]);
                auto pid = (pidHigh << 8) + pidLow;

                if (_filters[pid % sizeof(_filters)])
                {
                    // TS Paket übernehmen.
                    ::memcpy(dest, source, TSPACKETSIZE);

                    dest += TSPACKETSIZE;
                }

                // Nächstes Paket.
                source += TSPACKETSIZE;
            }

        // Angebrochenes Paket übernehmen.
        prev = end - source;

        if (prev > 0)
            ::memcpy(buffer, source, prev);

        // An den Client durchreichen.
        if (dest > payload)
            _frontend.sendResponse(payload, dest - payload);
    }

    // Übergabebereich kann nun wieder freigegeben werden.
    ::free(payload);

#ifdef DEBUG
    // Protokollausgabe.
    ::printf("-feeder (%ld bytes) (%d/%ld)\n", total, overflow, skipped);
#endif
}

/*
    Beendet die Entgegennahme von Nutzdaten für diesen PID.
*/
void Filter::stop()
{
    // Dateizugriff beenden.
    const auto fd = _fd;

    if (fd < 0)
        return;

    _fd = -1;

#ifdef DEBUG
    // Protokollausgabe.
    ::printf("close filter %d\n", fd);
#endif

    ::close(fd);

    // Entgegennahme der Daten beenden.
    ThreadTools::join(_thread);

#ifdef DEBUG
    // Protokollausgabe.
    ::printf("filter stopped\n");
#endif
}

/*
    Erstellt einen Dateizugriff auf den Demultiplexer.
*/
bool Filter::open()
{
    // Immer zuerst alles stoppen.
    stop();

    // Das dürfen wir gar nicht mehr.
    if (!_active)
        return false;

    // Dateihanlde anlegen.
    char path[40];

    ::sprintf(path, "/dev/dvb/adapter%i/demux%i", _frontend.adapter, _frontend.frontend);

    _fd = ::open(path, O_RDWR);

    if (_fd < 0)
        return false;

    // Vergrößerten Zwischenspeicher anlegen.
    ::ioctl(_fd, DMX_SET_BUFFER_SIZE, 10 * 1024 * 1024);

    // Datenstrom beim Demultiplexer anmelden.
    dmx_pes_filter_params filter = {
        0x2000,
        dmx_input::DMX_IN_FRONTEND,
        dmx_output::DMX_OUT_TAP,
        dmx_ts_pes::DMX_PES_OTHER,
        DMX_IMMEDIATE_START};

    if (::ioctl(_fd, DMX_SET_PES_FILTER, &filter) != 0)
    {
        // Entgegennahme der Daten nicht möglich.
        stop();

        return false;
    }

    // Entgegennahme der Daten und Weitergabe aktivieren.
    startThread();

    // Ergebnis melden.
    return true;
}

/*
    Startet einmalig die Entgegennahme der Daten.
*/
void Filter::startThread()
{
    if (!_thread)
        _thread = new std::thread(&Filter::feeder, this);
}
