#include "frontend.hpp"

#include "manager.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>

/*
    Verbindet ein nicht konfigurierte Verwaltung eines Frontends
    mit einem Dateihandle.
*/
bool Frontend::processConnect()
{
    // Gewünschtes Frontend einlesen.
    connect_request request;

    if (!readblock(&request, sizeof(request)))
    {
        return false;
    }

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

    // Spannung setzen.
    auto voltage_err = ::ioctl(_fd, FE_SET_VOLTAGE, SEC_VOLTAGE_13);

#ifdef DEBUG
    // Protokollierung.
    if (voltage_err != 0)
    {
        ::printf("can't set LNB voltage: %d (%d)\n", voltage_err, errno);
    }
#endif

    // Ständige Signalüberwachung aktivieren.
    _status = new std::thread(&Frontend::readStatus, this);

    return true;
}
