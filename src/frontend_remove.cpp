#include "frontend.hpp"

// Beendet die Entgegennahme eines einzelnen Datenstroms.
bool Frontend::processRemoveFilter()
{
    // Datenstromkennung auslesen.
    __u16 pid;

    if (!readblock(&pid, sizeof(pid)))
    {
        return false;
    }

    // Nutzung der Verwaltung wurde bereits beendet.
    if (!_active)
    {
        return false;
    }

    // Entgegennahme beendet.
    removeFilter(pid);

    return true;
}
