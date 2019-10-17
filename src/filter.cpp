#include "filter.hpp"
#include "frontend.hpp"

#include <fcntl.h>
#include <unistd.h>

/*
    Liest einen Datenstrom (einzelne PID) vom Demultiplexer und meldet
    das Ergebnis an die Frontendinstanz zur Weitergabe an den Client.
*/
void Filter::feeder()
{
#ifdef DEBUG
    // Protokollausgabe.
    ::printf("+pid=%d\n", _pid);
#endif

    // Die Größe des Zwischenspeichers orientiert sich an der Art des Datenstroms.
    auto bufsize = _type == frontend_response::section ? 1000 : DVBNET_FILTER_STREAM_BUFFER;

    // Zusätzlich zu den Nutzdaten muss auch immer die Kontrollstruktur aufgesetzt werden.
    response *data = reinterpret_cast<response *>(::malloc(sizeof(response) + bufsize));

    // Kontrollstruktur aufbereiten.
    data->type = _type;
    data->pid = _pid;

    // Nutzdatenbereich ermitteln.
    auto buffer = data->payload;

    for (;;)
    {
        // Daten aus dem Demultiplexer auslesen.
        auto bytes = ::read(_fd, buffer, bufsize);

        // Sobald keine Daten mehr ankommen wird das Auslesen beendet.
        if (bytes <= 0)
        {
            break;
        }

        // An den Client durchreichen.
        _frontend.sendResponse(data, bytes);
    }

    // Übergabebereich kann nun wieder freigegeben werden.
    ::free(data);

#ifdef DEBUG
    // Protokollausgabe.
    ::printf("-pid=%d\n", _pid);
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
    {
        return;
    }

    _fd = -1;

    ::close(fd);

    // Entgegennahme der Daten beenden.
    const auto thread = _thread;

    if (!thread)
    {
        return;
    }

    _thread = nullptr;

    try
    {
        thread->join();
    }
    catch (...)
    {
        ::printf("join failed\n");
    }

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
    {
        return false;
    }

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
    {
        _thread = new std::thread(&Filter::feeder, this);
    }
}
