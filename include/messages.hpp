#ifndef _DVBNET_MESSAGES_H
#define _DVBNET_MESSAGES_H 1

#include "diseqc.hpp"

extern "C"
{
#include <libdvbv5/dvb-frontend.h>
}

// Beschreibt einen Transponder.
struct SatelliteTune
{
    // Satellitensteuerung.
    diseqc_modes lnbMode;
    // Transponderbeschreibung.
    fe_modulation modulation;
    __u32 frequency;
    __u32 symbolrate;
    bool horizontal;
    fe_code_rate innerFEC;
    bool s2;
    fe_rolloff rolloff;
};

// Mögliche Befehl des Clients.
enum frontend_request
{
    add_filter = 0,
    connect_adapter = 1,
    del_all_filters = 2,
    del_filter = 3,
    tune = 4,
};

#endif