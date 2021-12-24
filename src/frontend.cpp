#include "frontend.hpp"

#include "filter.hpp"
#include "manager.hpp"
#include "threadTools.hpp"

// Erstellt eine neue Verwaltung für ein Frontend.
Frontend::Frontend(int tcp, FrontendManager *manager)
    : _active(true),
      _fd(-1),
      _filter(nullptr),
      _manager(manager),
      _tcp(tcp),
      adapter(0),
      frontend(0)
{
    // Zwischenspeicher konfigurieren.
    auto buffer = 10 * 1024 * 1024;

    setsockopt(_tcp, SOL_SOCKET, SO_SNDBUF, &buffer, sizeof(buffer));

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
        return;

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
            return false;

        // Speicher nachführen.
        cbuffer += read;
        len -= read;
    }

    return true;
}

// Nimmt Steuerbefehle vom Client entgegen.
void Frontend::waitRequest()
{
    ThreadTools::signal();

#ifdef DEBUG
    // Protokollierung.
    printf("+%d/%d client\n", adapter, frontend);
#endif

    for (;;)
    {
        // Nächsten Steuerbefehl einlesen.
        frontend_request request;

        if (!readblock(&request, sizeof(request)))
            break;

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
        case frontend_request::add_filter:
            ok = processAddFilter();
            break;
        case frontend_request::del_filter:
            ok = processRemoveFilter();
            break;
        case frontend_request::del_all_filters:
            ok = processRemoveAllFilters();
            break;
        }

        if (!ok)
            break;
    }

    // Verbindung schliessen.
    if (_active)
        close(true);

#ifdef DEBUG
    // Protokollierung.
    printf("-%d/%d client\n", adapter, frontend);
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

        stopFilter();

        ::close(fd);
    }

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
    printf("%d/%d ended\n", adapter, frontend);
#endif
}

void Frontend::stopFilter()
{
    auto filter = _filter;

    if (!filter)
        return;

    _filter = nullptr;

    delete filter;
}