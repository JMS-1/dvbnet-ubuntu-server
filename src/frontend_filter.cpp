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

    // Filter einmalig anlegen.
    if (!_filter)
    {
        _filter = new Filter(*this);

        if (!_filter->open())
            return false;
    }

    // Aktivieren.
    _filter->setFilter(pid, true);

    return true;
}
