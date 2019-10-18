#ifndef _DVBNET_FRONTEND_H
#define _DVBNET_FRONTEND_H 1

#include <unistd.h>

#include <map>
#include <mutex>
#include <thread>

#include "messages.hpp"

class Filter;
class FrontendManager;

// Hilfsklasse zum Sperren innerhalb eines Blocks.
class Locker
{
public:
    Locker(std::mutex &lock) : _lock(lock) { _lock.lock(); }
    ~Locker() { _lock.unlock(); }

private:
    std::mutex &_lock;
};

// Verwaltet ein einzelnes Frontend.
class Frontend
{
    friend class Filter;
    friend class FrontendManager;

public:
    // Legt ein Frontend zu einem Steuerdatenstrom an.
    Frontend(int tcp, FrontendManager *_manager);
    // Beendet das Frontend.
    ~Frontend();

private:
    // Gesetzt während die Verwaltung verwendet werden darf.
    volatile bool _active;
    // Zugehörige Frontendverwaltung.
    FrontendManager *_manager;
    // Verbindung zum Frontend.
    volatile int _fd;
    // Steuerkanal zum Client.
    volatile int _tcp;
    // Zu verwendender DVB Adapter.
    int adapter;
    // Zu verwendendes Frontend des DVB Adapters.
    int frontend;
    // Alle angemeldeten Datenströme.
    std::map<__u16, Filter *> _filters;
    // Entgegennahme von Steuerbefehlen des Clients.
    std::thread *_listener;
    // Überwachung des Empfangssignals.
    std::thread *_status;
    // Synchronisation der Datenstrukturen.
    std::mutex _lock;
    // Synchronisation der Kommunikation.
    std::mutex _client;

private:
    // Überwachung des Empfangssignals.
    void readStatus();
    // Liest Steuerdaten des Clients ein.
    bool readblock(void *buffer, int len);
    // Wertet Steuerdaten des Clients aus.
    void waitRequest();
    // Verbindet das Frontend einmalig mit der Hardware.
    bool processConnect();
    // Wechselt den Transponder.
    bool processTune();
    // Aktiviert den Empfang von Steuerdaten.
    bool processAddSection();
    // Aktiviert den Empfang von Nutzdaten.
    bool processAddStream();
    // Deaktiviert den Datenempfang eines Datenstroms.
    bool processRemoveFilter();
    // Deaktiviert den Datenempfang alles Datenströme.
    bool processRemoveAllFilters();
    // Beendet diese Verwaltung.
    void close(bool nowait);
    // Entfernt den Empfang eines einzelnen Datenstroms.
    void removeFilter(__u16 pid);
    // Beendet den Empfang alles Datenströme.
    void removeAllFilters();
    // Sendet Nutzdaten an den Client.
    void sendResponse(response *data, int payloadSize);
};

#endif