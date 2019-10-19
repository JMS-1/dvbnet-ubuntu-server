#ifndef _DVBNET_MANAGER_H
#define _DVBNET_MANAGER_H 1

#include <netinet/in.h>

#include <list>
#include <map>
#include <mutex>
#include <thread>

#include "frontend.hpp"

// Verwaltet Clientverbedingungen und damit auch die zugehörige Hardware.
class FrontendManager
{
    friend class Frontend;

public:
    FrontendManager();
    ~FrontendManager();

private:
    // Beendet die Verwaltung als Ganzes.
    void close();

private:
    // Alle bekannten Verbindungen.
    std::map<int, Frontend *> _frontends;
    // Alle vorhandenen Frontends.
    std::list<int> _available;
    // Überwachung neuer Verbindungen.
    std::thread *_listen;
    // Gesetzt während die Verwaltung aktiv ist.
    volatile bool _active;
    // Socket für die Entgegennahme von neuen Verbindungen.
    volatile int _fd;
    // Synchronisation der Verwaltungstrkturen.
    std::mutex _lock;

public:
    // Beginnt mit der Entgegennahme neuer Verbindungen.
    bool listen(in_port_t = 29713);
    // Aktiviert die Verwaltung als Server.
    void run() { _listen->join(); }

private:
    // Fügt eine Verbindungen zur Verwaltung hinzu.
    bool addFrontend(Frontend *frontend);
    // Entfernt eine Verbindung aus der Verwaltung.
    void removeFrontend(int adapter, int frontend);
    // Erwartet neue Verbindungen.
    void listener();
    // Reserviert eine freie Karte.
    void allocate(int &adapter, int &frontend);
};

#endif