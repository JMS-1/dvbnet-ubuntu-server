#include "frontend.hpp"

#include "streamFilter.hpp"

// Aktiviert den Empfang von Nutzdaten.
bool Frontend::processAddStream()
{
    // Datenstromkennung auslesen.
    __u16 pid;

    if (!readblock(&pid, sizeof(pid)))
    {
        return false;
    }

    // Verwaltung ist bereits beendet.
    if (!_active)
    {
        return false;
    }

    // Immer den aktuellen Empfang deaktivieren.
    removeFilter(pid);

    // Empfangsverwaltung anlegen und vermerken.
    auto filter = new StreamFilter(*this, pid);

    _filters[pid] = filter;

    // Entgegennahme der Nutzdaten aktivieren - Fehlerbehandlung ist bewußt deaktiviert.
    filter->start();

    return true;
}
