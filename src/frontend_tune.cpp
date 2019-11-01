#include "frontend.hpp"

#include "filter.hpp"

#include <sys/ioctl.h>

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
    if (_fd < 0)
        return false;

    // Aktiven Filter deaktivieren.
    stopFilter();

    // DiSEqC Steuerung durchführen.
    auto useSwitch = (transponder.lnbMode >= diseqc_modes::diseqc1) && (transponder.lnbMode <= diseqc_modes::diseqc4);
    auto hiFreq = useSwitch && transponder.frequency >= transponder.lnbSwitch;
    auto loFreq = useSwitch && transponder.frequency < transponder.lnbSwitch;
    auto freq = transponder.frequency - (hiFreq ? transponder.lnb2 : 0) - (loFreq ? transponder.lnb1 : 0);

    DiSEqCMessage diseqc(DiSEqCMessage::create(transponder.lnbMode, hiFreq, transponder.horizontal));

    // Umschaltung vornehmen - Fehlerbehandlung explizit deaktiviert.
    diseqc.send(_fd);

    // Transponder anwählen.
    struct dtv_property props[] =
        {
            {DTV_DELIVERY_SYSTEM, {0}, transponder.s2 ? fe_delivery_system::SYS_DVBS2 : fe_delivery_system::SYS_DVBS},
            {DTV_FREQUENCY, {0}, freq},
            {DTV_INNER_FEC, {0}, transponder.innerFEC},
            {DTV_MODULATION, {0}, transponder.modulation},
            {DTV_SYMBOL_RATE, {0}, transponder.symbolrate},
            {DTV_ROLLOFF, {0}, transponder.rolloff},
            {DTV_TUNE, {0}, 0}};

    struct dtv_properties dtv_prop = {
        .num = sizeof(props) / sizeof(props[0]), .props = props};

    // Fehlerbehandlung bewußt deaktiviert.
    auto tune_err = ::ioctl(_fd, FE_SET_PROPERTY, &dtv_prop);

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
