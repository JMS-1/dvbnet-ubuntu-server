#include "frontend.hpp"

#include "sectionFilter.hpp"

// Aktiviert den Empfang von Kontrolldaten.
bool Frontend::processAddSection()
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

    // Aktuellen Empfang deaktivieren.
    removeFilter(pid);

    // Empfang anlegen und vermerken.
    auto filter = new SectionFilter(*this, pid);

    _filters[pid] = filter;

    // Entgegennahme aktivieren - es wird bewußt auf eine Fehlerauswertung verzichtet.
    filter->start();

    return true;
}
