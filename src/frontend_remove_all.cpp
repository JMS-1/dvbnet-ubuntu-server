#include "frontend.hpp"

#include "filter.hpp"

// Beendet die Entgegennahme aller Datenströme.
bool Frontend::processRemoveAllFilters()
{
    // Nutzung der Verwaltung wurde bereits beendet.
    if (!_active)
    {
        return false;
    }

    // Entgegennahme beenden.
    if (_filter)
        _filter->clearFilter();

#ifdef DEBUG
    // Protokollierung.
    ::printf("-all filters\n");
#endif

    return true;
}
