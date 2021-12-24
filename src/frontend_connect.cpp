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

    // Dateizugriff erstellen.
    char path[40];

    ::sprintf(path, "/dev/dvb/adapter%i/frontend%i", adapter, frontend);

    _fd = ::open(path, O_RDWR);

    if (_fd < 0)
    {
        return false;
    }

    // Anmeldung bei der Frontendverwaltung.
    if (!manager->addFrontend(this))
    {
        return false;
    }

#ifdef DEBUG
    // Protokollierung.
    ::printf("%d/%d connected\n", adapter, frontend);
#endif

    return true;
}
