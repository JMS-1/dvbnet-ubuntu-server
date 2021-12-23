#include "filter.hpp"

#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <malloc.h>

#include "frontend.hpp"
#include "threadTools.hpp"

extern "C"
{
#include <libdvbv5/dvb-demux.h>
}

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

    while (_demux >= 0)
    {
        // Daten aus dem Demultiplexer auslesen.
        auto bytes = read(_demux, buffer + prev, BUFFERSIZE);

        // Sobald keine Daten mehr ankommen wird das Auslesen beendet.
        if (bytes < 0)
        {
            if (errno == EAGAIN)
            {
                // Wir sind einfach nur zu schnell.
                usleep(10000);
            }
            else if (errno == EOVERFLOW)
            {
                // Wir sind einfach nur zu langsam.
                overflow++;
            }
            else
            {
                // Hier ist wirklich etwas schief gegangen.
                if (errno != EBADF)
                    printf("read error: %d\n", errno);

                break;
            }

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
    const auto fd = _demux;

    if (fd < 0)
        return;

    _demux = -1;

#ifdef DEBUG
    // Protokollausgabe.
    printf("close filter %d\n", fd);
#endif

    // Alle Filter schon mal deaktivieren.
    clearFilter();

    // Entgegennahme der Daten beenden.
    ThreadTools::join(_thread);

    // Verbindung zum Gerät beenden.
    dvb_dmx_close(fd);

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
    _demux = dvb_dmx_open(_frontend.adapter, _frontend.frontend);

    if (_demux < 0)
    {
        printf("unable to open demux %d/%d: %d\n", _frontend.adapter, _frontend.frontend, errno);

        return false;
    }

    // Datenstrom beim Demultiplexer anmelden.
    if (dvb_set_pesfilter(_demux, 0x2000, DMX_PES_OTHER, DMX_OUT_TAP, 10 * 1024 * 1024))
    {
        printf("unable to set filter: %d\n", errno);

        // Entgegennahme der Daten nicht möglich.
        stop();

        return false;
    }

    // Entgegennahme der Daten und Weitergabe aktivieren.
    _thread = new std::thread(&Filter::feeder, this);

    // Ergebnis melden.
    return true;
}
