#ifndef _DVBNET_MANAGER_H
#define _DVBNET_MANAGER_H 1

#include <netinet/in.h>

#include <map>
#include <thread>

#include "frontend.hpp"

// Verwaltet Clientverbedingungen und damit auch die zugehörige Hardware.
class FrontendManager
{
    friend class Frontend;

public:
    FrontendManager() : _active(true), _fd(-1), _listen(nullptr) {}
    ~FrontendManager();

private:
    // Beendet die Verwaltung als Ganzes.
    void close();

private:
    // Alle bekannten Verbindungen.
    std::map<int, Frontend *> _frontends;
    // Überwachung neuer Verbindungen.
    std::thread *_listen;
    // Gesetzt während die Verwaltung aktiv ist.
    volatile bool _active;
    // Socket für die Entgegennahme von neuen Verbindungen.
    volatile int _fd;

public:
    // Beginnt mit der Entgegennahme neuer Verbindungen.
    bool listen(in_port_t = 29713);

private:
    // Fügt eine Verbindungen zur Verwaltung hinzu.
    bool addFrontend(Frontend *frontend);
    // Entfernt eine Verbindung aus der Verwaltung.
    void removeFrontend(int adapter, int frontend);
    // Erwartet neue Verbindungen.
    void listener();
};

#endif