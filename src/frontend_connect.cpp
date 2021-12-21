#include "frontend.hpp"

#include "filter.hpp"
#include "manager.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>

/*
    Verbindet ein nicht konfigurierte Verwaltung eines Frontends
    mit einem Dateihandle.
*/
bool Frontend::processConnect()
{
    // Verwaltung wird bereits beendet.
    if (!_active)
    {
        return false;
    }

    // Freies Frontend ermitteln.
    auto manager = _manager;

    if (!manager)
    {
        return false;
    }

    manager->allocate(adapter, frontend);

    // Zugriff zum Gerät erstellen.
    _fe = dvb_fe_open(adapter, frontend, 0, 0);

    if (!_fe)
    {
        return false;
    }

    // Anmeldung bei der Frontendverwaltung.
    if (!manager->addFrontend(this))
    {
        return false;
    }

    return true;
}
