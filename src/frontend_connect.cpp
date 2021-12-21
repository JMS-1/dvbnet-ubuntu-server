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

    // Spannung setzen.
    auto voltage_err = dvb_fe_sec_voltage(const_cast<dvb_v5_fe_parms *>(_fe), 1, 0);

#ifdef DEBUG
    // Protokollierung.
    if (voltage_err != 0)
    {
        ::printf("can't set LNB voltage: %d (%d)\n", voltage_err, errno);
    }
#endif

#ifdef DEBUG
    // Protokollierung.
    ::printf("%d/%d connected\n", adapter, frontend);
#endif

    // Standard LNB Konfiguration zuweisen.
    _fe->lnb = dvb_sat_get_lnb(dvb_sat_search_lnb("EXTENDED"));

    return true;
}
