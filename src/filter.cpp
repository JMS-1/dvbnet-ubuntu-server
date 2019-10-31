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
    ::printf("+pid=%d\n", _pid);
#endif

    // Die Größe des Zwischenspeichers orientiert sich an der Art des Datenstroms.
    const auto bufsize = 100 * TSPACKETSIZE;

    // Zusätzlich zu den Nutzdaten muss auch immer die Kontrollstruktur aufgesetzt werden.
    response *data = reinterpret_cast<response *>(::malloc(sizeof(response) + 2 * (bufsize + TSPACKETSIZE)));

    // Kontrollstruktur aufbereiten.
    data->type = _type;
    data->pid = _pid;

    // Nutzdatenbereich ermitteln.
    auto payload = data->payload;
    auto buffer = payload + bufsize + TSPACKETSIZE;
    auto prev = 0;

    auto overflow = 0;
    auto skipped = 0;
    ssize_t total = 0;

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
                // TS Paket übernehmen.
                ::memcpy(dest, source, TSPACKETSIZE);

                dest += TSPACKETSIZE;

                // Nächstes Paket.
                source += TSPACKETSIZE;
            }

        // Angebrochenes Paket übernehmen.
        prev = end - source;

        if (prev > 0)
            ::memcpy(buffer, source, prev);

        // An den Client durchreichen.
        if (dest > payload)
            _frontend.sendResponse(data, dest - payload);
    }

    // Übergabebereich kann nun wieder freigegeben werden.
    ::free(data);

#ifdef DEBUG
    // Protokollausgabe.
    ::printf("-pid=%d (%ld bytes) (%d/%d)\n", _pid, total, overflow, skipped);
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
    ::printf("close pid %d fd %d\n", _pid, fd);
#endif

    ::close(fd);

    // Entgegennahme der Daten beenden.
    ThreadTools::join(_thread);

#ifdef DEBUG
    // Protokollausgabe.
    ::printf("%d stopped\n", _pid);
#endif
}

/*
    Erstellt einen Dateizugriff auf den Demultiplexer.
*/
int Filter::open()
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

    // Ergebnis melden.
    return _fd;
}

/*
    Startet einmalig die Entgegennahme der Daten.
*/
void Filter::startThread()
{
    if (!_thread)
        _thread = new std::thread(&Filter::feeder, this);
}
