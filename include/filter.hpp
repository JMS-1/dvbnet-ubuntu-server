#ifndef _DVBNET_FILTER_H
#define _DVBNET_FILTER_H 1

#include <thread>

#include <string.h>
#include <linux/types.h>
#include <sys/types.h>

class Frontend;

/*
    Verwaltet die Entgegennahme eines Datenstroms.
*/
class Filter
{
public:
    Filter(const Frontend &frontend) : _fd(-1), _frontend(frontend), _thread(nullptr) { clearFilter(); }
    ~Filter() { stop(); }

private:
    // Alle möglichen Datenströme.
    u_char _filters[0x2000];
    // Zugehöriges Frontend.
    const Frontend &_frontend;
    // Dateihandle zum Demultiplexer.
    volatile int _fd;
    // Instanz für die Datenweitergabe.
    std::thread *_thread;

private:
    // Gibt Daten an den Client weiter.
    void feeder();
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