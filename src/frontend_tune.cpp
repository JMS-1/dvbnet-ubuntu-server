#include "frontend.hpp"

#include "filter.hpp"

#include <sys/ioctl.h>
#include <stdio.h>

// Frontend auf einen neuen Transponder ausrichten.
bool Frontend::processTune()
{
    // Transponder auslesen.
    SatelliteTune transponder;

    if (!readblock(&transponder, sizeof(transponder)))
        return false;

    // Verwaltung ist bereits beendet.
    if (!_active)
        return false;

    // Verwaltung ist noch nicht mit einem Frontend verbunden.
    if (!_fe)
        return false;

    // Aktiven Filter deaktivieren.
    stopFilter();

    // Umschaltung vornehmen - Fehlerbehandlung explizit deaktiviert.
    //diseqc.send(_fd);

    // Transponder anwählen.
    dvb_fe_store_parm(const_cast<dvb_v5_fe_parms *>(_fe), DTV_DELIVERY_SYSTEM, transponder.s2 ? fe_delivery_system::SYS_DVBS2 : fe_delivery_system::SYS_DVBS);
    dvb_fe_store_parm(const_cast<dvb_v5_fe_parms *>(_fe), DTV_FREQUENCY, transponder.frequency);
    dvb_fe_store_parm(const_cast<dvb_v5_fe_parms *>(_fe), DTV_MODULATION, transponder.modulation);
    dvb_fe_store_parm(const_cast<dvb_v5_fe_parms *>(_fe), DTV_SYMBOL_RATE, transponder.symbolrate);
    dvb_fe_store_parm(const_cast<dvb_v5_fe_parms *>(_fe), DTV_INNER_FEC, transponder.innerFEC);
    dvb_fe_store_parm(const_cast<dvb_v5_fe_parms *>(_fe), DTV_ROLLOFF, transponder.rolloff);

    // Fehlerbehandlung bewußt deaktiviert.
    auto tune_err = dvb_fe_set_parms(const_cast<dvb_v5_fe_parms *>(_fe));

#ifdef DEBUG
    // Protokollierung.
    if (tune_err != 0)
        ::printf("can't tune: %d (%d)\n", tune_err, errno);
#endif

    // Eine kleine Pause um sicherzustellen, dass der Vorgang auch abgeschlossen wurde.
    ::sleep(2);

    // Filter jetzt erzeugen.
    _filter = new Filter(*this);

    if (!_filter->open())
        return false;

#ifdef DEBUG
    // Protokollierung.
    ::printf("%d/%d tuned\n", adapter, frontend);
#endif

    return true;
}
