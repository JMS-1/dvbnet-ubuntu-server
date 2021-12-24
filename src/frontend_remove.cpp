#include <stdio.h>

#include "frontend.hpp"

#include "filter.hpp"

// Beendet die Entgegennahme eines einzelnen Datenstroms.
bool Frontend::processRemoveFilter()
{
    // Datenstromkennung auslesen.
    __u16 pid;

    if (!readblock(&pid, sizeof(pid)))
        return false;

    // Nutzung der Verwaltung wurde bereits beendet.
    if (!_active)
        return false;

    // Entgegennahme beendet.
    if (_filter)
        _filter->setFilter(pid, false);

#ifdef DEBUG
    // Protokollierung.
    printf("-filter %d\n", pid);
#endif

    return true;
}
