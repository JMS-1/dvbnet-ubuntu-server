#include "frontend.hpp"

#include "filter.hpp"

#include <sys/ioctl.h>
#include <stdio.h>

#define MIN_STATUS (FE_HAS_LOCK)

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
    if (voltage_err)
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
    else
        _fe->sat_number = -1;

    printf("%d %d %d\n", transponder.diseqc, transponder.horizontal, transponder.frequency >= fe->lnb->rangeswitch * 1000);

    // System setzen.
    if (dvb_set_sys(fe, transponder.s2 ? SYS_DVBS2 : SYS_DVBS))
        ::printf("failed to set delivery system: %d\n", errno);

    // Transponder anwählen.
    dvb_fe_store_parm(fe, DTV_POLARIZATION, transponder.horizontal ? POLARIZATION_H : POLARIZATION_V);
    dvb_fe_store_parm(fe, DTV_FREQUENCY, transponder.frequency);
    dvb_fe_store_parm(fe, DTV_INVERSION, INVERSION_AUTO);
    dvb_fe_store_parm(fe, DTV_SYMBOL_RATE, transponder.symbolrate);
    dvb_fe_store_parm(fe, DTV_INNER_FEC, transponder.innerFEC);

    if (transponder.s2)
    {
        dvb_fe_store_parm(fe, DTV_MODULATION, transponder.modulation);
        dvb_fe_store_parm(fe, DTV_ROLLOFF, transponder.rolloff);
        dvb_fe_store_parm(fe, DTV_PILOT, PILOT_AUTO);
    }

    // Fehlerbehandlung bewußt deaktiviert.
    auto tune_err = dvb_fe_set_parms(fe);

    // Protokollierung.
    if (tune_err)
    {
        printf("can't tune: %d (%d)\n", tune_err, errno);

        return false;
    }

    // Eine kleine Pause um sicherzustellen, dass der Vorgang auch abgeschlossen wurde.
    uint32_t status = 0;

    for (int end = ::time(nullptr) + 2; ::time(nullptr) < end; usleep(100000))
    {
        if (dvb_fe_get_stats(fe))
        {
            printf("can no get stats: %d\n", errno);

            break;
        }

        if (dvb_fe_retrieve_stats(fe, DTV_STATUS, &status))
        {
            printf("can no get status: %d\n", errno);

            break;
        }

        if ((status & MIN_STATUS) == MIN_STATUS)
            break;
    }

    if ((status & MIN_STATUS) != MIN_STATUS)
        printf("no lock %s %d%s: %d\n", transponder.s2 ? "S2" : "S", transponder.frequency, transponder.horizontal ? "H" : "V", status);

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
