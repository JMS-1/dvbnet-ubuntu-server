#include "frontend.hpp"

#include "filter.hpp"

// Aktiviert den Empfang von Kontrolldaten.
bool Frontend::processAddFilter()
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

    // Aktivieren.
    if (_filter)
        _filter->setFilter(pid, true);

#ifdef DEBUG
    // Protokollierung.
    ::printf("+filter %d\n", pid);
#endif

    return true;
}
