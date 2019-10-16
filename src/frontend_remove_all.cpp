#include "frontend.hpp"

// Beendet die Entgegennahme aller Datenströme.
bool Frontend::processRemoveAllFilters()
{
    removeAllFilters();

    return true;
}
