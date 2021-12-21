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
    auto fe = const_cast<dvb_v5_fe_parms *>(_fe);

    if (!fe)
        return false;

    // Aktiven Filter deaktivieren.
    stopFilter();

    // Spannung setzen.
    auto voltage_err = dvb_fe_sec_voltage(fe, transponder.lnbPower ? 1 : 0, 0);

    // Protokollierung.
    if (voltage_err != 0)
        printf("can't set LNB voltage: %d (%d)\n", voltage_err, errno);

#ifdef DEBUG
    // Protokollierung.
    printf("%d/%d connected\n", adapter, frontend);
#endif

    // Standard LNB Konfiguration zuweisen.
    _fe->lnb = dvb_sat_get_lnb(dvb_sat_search_lnb("EXTENDED"));

    // DiSEqC Steuerung durchführen.
    if (transponder.diseqc >= 1 && transponder.diseqc <= 4)
        _fe->sat_number = transponder.diseqc - 1;

    // Transponder anwählen.
    dvb_fe_store_parm(fe, DTV_DELIVERY_SYSTEM, transponder.s2 ? SYS_DVBS2 : SYS_DVBS);
    dvb_fe_store_parm(fe, DTV_FREQUENCY, transponder.frequency);
    dvb_fe_store_parm(fe, DTV_MODULATION, transponder.modulation);
    dvb_fe_store_parm(fe, DTV_POLARIZATION, transponder.horizontal ? POLARIZATION_H : POLARIZATION_V);
    dvb_fe_store_parm(fe, DTV_SYMBOL_RATE, transponder.symbolrate);
    dvb_fe_store_parm(fe, DTV_INNER_FEC, transponder.innerFEC);
    dvb_fe_store_parm(fe, DTV_ROLLOFF, transponder.rolloff);

    // Fehlerbehandlung bewußt deaktiviert.
    auto tune_err = dvb_fe_set_parms(fe);

    // Protokollierung.
    if (tune_err != 0)
        printf("can't tune: %d (%d)\n", tune_err, errno);

    // Eine kleine Pause um sicherzustellen, dass der Vorgang auch abgeschlossen wurde.
    for (int end = ::time(nullptr) + 2; ::time(nullptr) < end; usleep(100))
    {
        if (dvb_fe_get_stats(fe))
            break;

        uint32_t status = 0;

        if (dvb_fe_retrieve_stats(fe, DTV_STATUS, &status))
            break;

        if (status & FE_HAS_LOCK)
            break;
    }

    // Filter jetzt erzeugen.
    _filter = new Filter(*this);

    if (!_filter->open())
        return false;

#ifdef DEBUG
    // Protokollierung.
    printf("%d/%d tuned\n", adapter, frontend);
#endif

    return true;
}
