#include "manager.hpp"
#include "threadTools.hpp"

#include <sys/socket.h>

// Überwacht eingehende Verbindung - jede Verbindung erstellt ein Frontend.
void FrontendManager::listener()
{
    ThreadTools::signal();

#ifdef DEBUG
    // Protokollierung.
    ::printf("+listen\n");
#endif

    for (;;)
    {
        // Neue Verbindung entgegennehmen.
        auto fd = ::accept(_fd, nullptr, nullptr);

        if (fd < 0)
        {
            break;
        }

        // Nicht initialisierte Verwaltung erstellen, alles weitere erledigen Steuerbefehle des Clients.
        new Frontend(fd, this);
    }

#ifdef DEBUG
    // Protokollierung.
    ::printf("-listen\n");
#endif
}

// Beginnt mit der Überwachung eingehender Verbindungen.
bool FrontendManager::listen(in_port_t port /* = 29713 */)
{
    // Vorherige Operationen beenden.
    close();

    // Socket anlegen.
    _fd = ::socket(AF_INET, SOCK_STREAM, 0);

    if (_fd < 0)
    {
        return false;
    }

    // Entgegennahme von Verbindungen vorbereiten.
    sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = ::htons(port),
        .sin_addr = {.s_addr = ::htonl(INADDR_ANY)},
        .sin_zero = {0}};

    if (::bind(_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0)
    {
        // Im Fehlerfall Verwaltung zurücksetzen.
        close();

        return false;
    }

    // Entgegennahme aktivieren.
    if (::listen(_fd, 5) != 0)
    {
        // Im Fehlerfall Verwaltung zurücksetzen.
        close();

        return false;
    }

    // Überwachung einrichten.
    _listen = new std::thread(&FrontendManager::listener, this);

    return true;
}

// Beenden der Verwaltung.
void FrontendManager::close()
{
    // Socket beenden.
    auto fd = _fd;

    if (fd < 0)
    {
        return;
    }

    _fd = -1;

    ::close(fd);

    // Überwachung beenden.
    ThreadTools::join(_listen);

#ifdef DEBUG
    // Protokollierung.
    ::printf("-manager\n");
#endif
}

// Beendet die Verwaltung.
FrontendManager::~FrontendManager()
{
    // Sperren.
    Locker _self(_lock);

    // Neue Verbindung sind nun nicht mehr gestattet.
    _active = false;

    if (_fd >= 0)
    {
        ::shutdown(_fd, SHUT_RD);
    }

    // Alle verwalteten Frontends beenden.
    for (auto &frontend : _frontends)
    {
        delete frontend.second;
    }

    // Verwaltung leeren.
    _frontends.clear();

    // Verwaltung beenden.
    close();
}

// Schlüssel zur einfachen Verwaltung von Verbindungen erstellen.
int makeKey(int adapter, int frontend)
{
    // Die Hardwaregrenze von maximal 1000 DVB Karten mit maximal 1000 Frontends sollte kein Problem sein.
    if (adapter < 0 || adapter > 999)
    {
        return -1;
    }

    if (frontend < 0 || frontend > 999)
    {
        return -1;
    }

    // Dann ist es auch sehr einfach einen Schlüssel zu erstellen.
    return 1000 * adapter + frontend;
}

// Trägte ein Frontend in die Verwaltung ein.
bool FrontendManager::addFrontend(Frontend *frontend)
{
    // Sperren.
    Locker _self(_lock);

    // Verwaltung ist bereits beendet.
    if (!_active)
    {
        return false;
    }

    // Eindeutigen Schlüssel erstellen.
    auto key = makeKey(frontend->adapter, frontend->frontend);

    if (key < 0)
    {
        return false;
    }

    // Pro Hardware darf es nur eine Verwaltung geben.
    if (_frontends.find(key) != _frontends.end())
    {
        return false;
    }

    // Verbindung vermerken.
    _frontends[key] = frontend;

    return true;
}

// Verbindung aus der Verwaltung entfernen.
void FrontendManager::removeFrontend(int adapter, int frontend)
{
    // Verwaltung ist bereits deaktiviert.
    if (!_active)
    {
        return;
    }

    // Sperren.
    Locker _self(_lock);

    // Verwaltung ist bereits deaktiviert.
    if (!_active)
    {
        return;
    }

    // Schlüssel anlegen.
    auto key = makeKey(adapter, frontend);

    if (key < 0)
    {
        return;
    }

    // Verbindung entfernen falls vorhanden.
    if (_frontends.find(key) != _frontends.end())
    {
        _frontends.erase(key);
    }
}