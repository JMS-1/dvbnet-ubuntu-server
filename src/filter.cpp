#include "filter.hpp"

#include <fcntl.h>
#include <string.h>
#include <linux/dvb/dmx.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <malloc.h>

#include "frontend.hpp"
#include "threadTools.hpp"

#define TSPACKETSIZE 188
#define TSMAGICBYTE 0x47

#define BUFFERSIZE (100 * TSPACKETSIZE)
#define SCRATCHSIZE (BUFFERSIZE + TSPACKETSIZE)
/*
    Liest einen Datenstrom (einzelne PID) vom Demultiplexer und meldet
    das Ergebnis an die Frontendinstanz zur Weitergabe an den Client.
*/
void Filter::feeder()
{
    ThreadTools::signal();

#ifdef DEBUG
    // Protokollausgabe.
    printf("+feeder\n");
#endif

    // Zusätzlich zu den Nutzdaten muss auch immer die Kontrollstruktur aufgesetzt werden.
    auto payload = reinterpret_cast<u_char *>(malloc(2 * SCRATCHSIZE));

    // Nutzdatenbereich ermitteln.
    auto buffer = payload + SCRATCHSIZE;
    auto prev = 0;

    ssize_t skipped = 0, total = 0;
    auto overflow = 0;

    for (;;)
    {
        // Daten aus dem Demultiplexer auslesen.
        auto bytes = read(_fd, buffer + prev, BUFFERSIZE);

        // Sobald keine Daten mehr ankommen wird das Auslesen beendet.
        if (bytes < 0)
        {
            // Es sei denn wir sind einfach nur zu langsam.
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
        auto source = buffer, end = source + bytes, dest = payload;

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
                auto pid = ((pidHigh << 8) + pidLow) % sizeof(_filters);

                if (_filters[pid])
                {
                    // TS Paket übernehmen.
                    memcpy(dest, source, TSPACKETSIZE);

                    dest += TSPACKETSIZE;
                }

                // Nächstes Paket.
                source += TSPACKETSIZE;
            }

        // Angebrochenes Paket übernehmen.
        prev = end - source;

        if (prev > 0)
            memcpy(buffer, source, prev);

        // An den Client durchreichen.
        auto send = dest - payload;

        total += send;

        if (send > 0)
            _frontend.sendResponse(payload, send);
    }

    // Übergabebereich kann nun wieder freigegeben werden.
    free(payload);

#ifdef DEBUG
    // Protokollausgabe.
    printf("-feeder (%ld bytes) (%d/%ld)\n", total, overflow, skipped);
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
    printf("close filter %d\n", fd);
#endif

    // Alle Filter schon mal deaktivieren.
    clearFilter();

    // Verbindung zum Gerät beenden.
    ::close(fd);

    // Entgegennahme der Daten beenden.
    ThreadTools::join(_thread);

#ifdef DEBUG
    // Protokollausgabe.
    printf("filter stopped\n");
#endif
}

/*
    Erstellt einen Dateizugriff auf den Demultiplexer.
*/
bool Filter::open()
{
    // Immer zuerst alles stoppen.
    stop();

    // Dateihanlde anlegen.
    char path[40];

    sprintf(path, "/dev/dvb/adapter%i/demux%i", _frontend.adapter, _frontend.frontend);

    _fd = ::open(path, O_RDWR);

    if (_fd < 0)
        return false;

    // Vergrößerten Zwischenspeicher anlegen.
    auto buf_err = ioctl(_fd, DMX_SET_BUFFER_SIZE, 10 * 1024 * 1024);

#ifdef DEBUG
    // Protokollierung.
    if (buf_err != 0)
        printf("can't set buffer: %d\n", errno);
#endif

    // Datenstrom beim Demultiplexer anmelden.
    dmx_pes_filter_params filter = {
        .pid = 0x2000,
        .input = dmx_input::DMX_IN_FRONTEND,
        .output = dmx_output::DMX_OUT_TAP,
        .pes_type = dmx_ts_pes::DMX_PES_OTHER,
        .flags = DMX_IMMEDIATE_START};

    if (ioctl(_fd, DMX_SET_PES_FILTER, &filter) != 0)
    {
        // Entgegennahme der Daten nicht möglich.
        stop();

        return false;
    }

    // Entgegennahme der Daten und Weitergabe aktivieren.
    _thread = new std::thread(&Filter::feeder, this);

    // Ergebnis melden.
    return true;
}
