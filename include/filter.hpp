#ifndef _DVBNET_FILTER_H
#define _DVBNET_FILTER_H 1

#include <thread>

#include <string.h>
#include <linux/types.h>

class Frontend;

/*
    Verwaltet die Entgegennahme eines Datenstroms.
*/
class Filter
{
public:
    // Erstellt eine neue Verwaltung für ein bestimmtes Frontend.
    Filter(Frontend &frontend)
        : _active(true),
          _fd(-1),
          _frontend(frontend),
          _thread(nullptr)
    {
        // Alle Filter zurücksetzen.
        clearFilter();
    }

    // Beendet diese Verwaltung.
    ~Filter()
    {
        // Sicherstellen, dass keine neuen Verwaltungen erzeugt werden.
        _active = false;

        // Endgültig beenden.
        stop();
    }

private:
    // Alle möglichen Datenströme.
    char _filters[0x2000];
    // Zugehöriges Frontend.
    Frontend &_frontend;
    // Gesetzt während die Verwaltung verwendet werden darf.
    volatile bool _active;
    // Dateihandle zum Demultiplexer.
    volatile int _fd;
    // Instanz für die Datenweitergabe.
    std::thread *_thread;

private:
    // Gibt Daten an den Client weiter.
    void feeder();
    // Beginnt mit der Entgegennahme der Nutzdaten.
    void startThread();
    // Beendet die Verwaltung.
    void stop();

public:
    // Filterung aktivieren oder deaktivieren.
    void setFilter(__u16 pid, bool enable) { _filters[pid % sizeof(_filters)] = enable ? 1 : 0; }
    // Alle Filter zurücksetzen.
    void clearFilter() { ::memset(_filters, 0, sizeof(_filters)); }
    // Erstellt einen Datenzugang zum Demultiplexer.
    bool open();
};

#endif