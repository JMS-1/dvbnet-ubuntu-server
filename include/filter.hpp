#ifndef _DVBNET_FILTER_H
#define _DVBNET_FILTER_H 1

#include <thread>

#include <linux/types.h>

#include "frontend.hpp"

/*
    Verwaltet die Entgegennahme eines Datenstroms.
*/
class Filter
{
    friend class Frontend;

protected:
    // Erstellt eine neue Verwaltung für ein bestimmtes Frontend.
    Filter(Frontend &frontend, __u16 pid, frontend_response type)
        : _active(true),
          _fd(-1),
          _frontend(frontend),
          _pid(pid),
          _thread(nullptr),
          _type(type)
    {
    }

protected:
    // Beendet diese Verwaltung.
    virtual ~Filter()
    {
        // Sicherstellen, dass keine neuen Verwaltungen erzeugt werden.
        _active = false;

        // Endgültig beenden.
        stop();
    }

private:
    // Zugehöriges Frontend.
    Frontend &_frontend;
    // Die Art des Datenstroms.
    const frontend_response _type;
    // Gesetzt während die Verwaltung verwendet werden darf.
    volatile bool _active;
    // Dateihandle zum Demultiplexer.
    int _fd;
    // Instanz für die Datenweitergabe.
    std::thread *_thread;

private:
    // Gibt Daten an den Client weiter.
    void feeder();

protected:
    // Die Identifikation (PID) des Datenstroms.
    const __u16 _pid;

protected:
    // Erstellt einen Datenzugang zum Demultiplexer.
    int open();
    // Beginnt mit der Entgegennahme der Nutzdaten.
    void startThread();
    // Beendet die Verwaltung.
    void stop();

public:
    // Beginnt mit der Entgegennahme und Weitergabe der Daten.
    virtual bool start() = 0;
};

#endif