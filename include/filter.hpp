#ifndef _DVBNET_FILTER_H
#define _DVBNET_FILTER_H 1

#include <thread>

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
    // Erstellt einen Datenzugang zum Demultiplexer.
    bool open();
};

#endif