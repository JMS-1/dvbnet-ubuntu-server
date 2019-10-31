#include "frontend.hpp"

#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "filter.hpp"
#include "manager.hpp"
#include "threadTools.hpp"

// Erstellt eine neue Verwaltung für ein Frontend.
Frontend::Frontend(int tcp, FrontendManager *manager)
    : _active(true),
      _fd(-1),
      _filter(nullptr),
      _manager(manager),
      _status(nullptr),
      _tcp(tcp),
      adapter(adapter),
      frontend(frontend)
{
    // Zwischenspeicher konfigurieren.
    auto buffer = 10 * 1024 * 1024;

    ::setsockopt(_tcp, SOL_SOCKET, SO_SNDBUF, &buffer, sizeof(buffer));

    // Beginnt mit der Überwachung von eingegenden Steuermeldungen.
    _listener = new std::thread(&Frontend::waitRequest, this);
}

// Vernichtet eine Verwaltung.
Frontend::~Frontend()
{
    // Neue Anfragen sind nun nicht mehr möglich.
    _active = false;

    // Verwaltung beenden.
    close(false);

    // An die Verwaltung aller Frontends melden.
    auto manager = _manager;

    if (!manager)
    {
        return;
    }

    _manager = nullptr;

    manager->removeFrontend(adapter, frontend);
}

// Kontrolldaten vom Client einlesen.
bool Frontend::readblock(void *buffer, int len)
{
    // Daten einlesen.
    for (auto cbuffer = static_cast<char *>(buffer); len > 0;)
    {
        // Nächste Daten einlesen.
        auto read = ::read(_tcp, cbuffer, len);

        // Verbindung wurde beendet.
        if (read <= 0)
        {
            return false;
        }

        // Speicher nachführen.
        cbuffer += read;
        len -= read;
    }

    return true;
}

// Nimmt Steuerbefehle vom Client entgegen.
void Frontend::waitRequest()
{
    ::ThreadTools::signal();

#ifdef DEBUG
    // Protokollierung.
    ::printf("+%d/%d client\n", adapter, frontend);
#endif

    for (;;)
    {
        // Nächsten Steuerbefehl einlesen.
        frontend_request request;

        if (!readblock(&request, sizeof(request)))
        {
            break;
        }

        // Steuerbefehl ausführen.
        auto ok = false;

        switch (request)
        {
        case frontend_request::connect_adapter:
            ok = processConnect();
            break;
        case frontend_request::tune:
            ok = processTune();
            break;
        case frontend_request::add_section_filter:
            ok = processAddSection();
            break;
        case frontend_request::add_stream_filter:
            ok = processAddStream();
            break;
        case frontend_request::del_filter:
            ok = processRemoveFilter();
            break;
        case frontend_request::del_all_filters:
            ok = processRemoveAllFilters();
            break;
        }

        if (!ok)
        {
            break;
        }
    }

    // Verbindung schliessen.
    if (_active)
    {
        close(true);
    }

#ifdef DEBUG
    // Protokollierung.
    ::printf("-%d/%d client\n", adapter, frontend);
#endif

    // Instanz vernichten.
    delete this;
}

// Beendet die Verwaltung implizit nach einem Fehler mit dem Steuerkanal.
void Frontend::close(bool nowait)
{
    // Dateihandle zum Frontend schliessen.
    auto fd = _fd;

    if (fd >= 0)
    {
        _fd = -1;

        // Alle angemeldeten Filter beenden.
        removeAllFilters();

        ::close(fd);
    }

    // Signalüberwachung beenden.
    ThreadTools::join(_status);

    // Steuerkanal schliessen.
    auto tcp = _tcp;

    if (tcp >= 0)
    {
        _tcp = -1;

        ::close(tcp);

        // Überwachung des Steuerkanals beendet.
        ThreadTools::join(_listener, nowait);
    }

#ifdef DEBUG
    // Protokollierung.
    ::printf("%d/%d ended\n", adapter, frontend);
#endif
}

// Überwacht das Empfangssignal.
void Frontend::readStatus()
{
    ::ThreadTools::signal();

#ifdef DEBUG
    // Protokollierung.
    ::printf("+status %d/%d\n", adapter, frontend);
#endif

    // Protokollstruktur anlegen.
    response *data = reinterpret_cast<response *>(::malloc(sizeof(response) + sizeof(signal_response)));

    // Protokollstruktur vorbereiten.
    data->type = frontend_response::signal_status;
    data->pid = 0;

    // Zugriff auf die Signaldaten.
    auto &signal = *reinterpret_cast<signal_response *>(data->payload);

    for (;; ::sleep(5))
    {
        // Detaildaten auslesen.
        if (::ioctl(_fd, FE_READ_STATUS, &signal.status) != 0)
        {
            break;
        }

        if (::ioctl(_fd, FE_READ_SIGNAL_STRENGTH, &signal.strength) != 0)
        {
            break;
        }

        if (::ioctl(_fd, FE_READ_SNR, &signal.snr) != 0)
        {
            break;
        }

        if (::ioctl(_fd, FE_READ_BER, &signal.ber) != 0)
        {
            break;
        }

        // Informationen an den Client melden.
        sendResponse(data, sizeof(signal_response));
    }

    // Protokollstruktur freigeben.
    ::free(data);

#ifdef DEBUG
    // Protokollierung.
    ::printf("-status %d/%d\n", adapter, frontend);
#endif
}

// Einzelnen Datenempfang beenden.
void Frontend::removeFilter(__u16 pid)
{
    // Synchronisation.
    Locker _self(_lock);
}

// Gesamten Datenempfang beenden.
void Frontend::removeAllFilters()
{
    // Synchronisation.
    Locker _self(_lock);

    auto filter = _filter;

    if (filter)
    {
        _filter = nullptr;

        delete filter;
    }
}

// Sendet Daten an den Client.
void Frontend::sendResponse(response *data, int payloadSize)
{
    // Nutzdaten festlegen.
    data->len = payloadSize;

    // Gesamte Länger berechnen.
    auto bytes = sizeof(response) + payloadSize;

    // Kontroll- und Steuerdaten als Einheit senden.
    Locker _self(_client);

    ::send(_tcp, data, bytes, MSG_NOSIGNAL | MSG_DONTWAIT);
}
