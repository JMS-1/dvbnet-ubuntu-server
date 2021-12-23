#ifndef _DVBNET_FRONTEND_H
#define _DVBNET_FRONTEND_H 1

#include <unistd.h>
#include <netinet/in.h>

#include <map>
#include <thread>

#include "messages.hpp"

extern "C"
{
#include <libdvbv5/dvb-fe.h>
}

class Filter;
class FrontendManager;

// Verwaltet ein einzelnes Frontend.
class Frontend
{
public:
    // Legt ein Frontend zu einem Steuerdatenstrom an.
    Frontend(int tcp, FrontendManager *_manager);
    // Beendet das Frontend.
    ~Frontend();

public:
    // Zu verwendender DVB Adapter.
    int adapter;
    // Zu verwendendes Frontend des DVB Adapters.
    int frontend;

public:
    // Sendet Nutzdaten an den Client.
    void sendResponse(const void *data, int bytes) const { ::send(_tcp, data, bytes, MSG_NOSIGNAL | MSG_DONTWAIT); }

private:
    // Gesetzt während die Verwaltung verwendet werden darf.
    volatile bool _active;
    // Zugehörige Frontendverwaltung.
    FrontendManager *_manager;
    // Verbindung zum Frontend.
    volatile dvb_v5_fe_parms *_fe;
    // Steuerkanal zum Client.
    volatile int _tcp;
    // Alle angemeldeten Datenströme.
    Filter *_filter;
    // Entgegennahme von Steuerbefehlen des Clients.
    std::thread *_listener;

private:
    // Liest Steuerdaten des Clients ein.
    bool readblock(void *buffer, int len);
    // Wertet Steuerdaten des Clients aus.
    void waitRequest();
    // Verbindet das Frontend einmalig mit der Hardware.
    bool processConnect();
    // Wechselt den Transponder.
    bool processTune();
    // Aktiviert den Empfang von Daten.
    bool processAddFilter();
    // Deaktiviert den Datenempfang eines Datenstroms.
    bool processRemoveFilter();
    // Deaktiviert den Datenempfang alles Datenströme.
    bool processRemoveAllFilters();
    // Beendet diese Verwaltung.
    void close(bool nowait);
    // Benden die Entgegennahme der Daten.
    void stopFilter();
};

#endif