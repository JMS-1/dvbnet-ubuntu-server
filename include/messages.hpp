#ifndef _DVBNET_MESSAGES_H
#define _DVBNET_MESSAGES_H 1

#include <linux/dvb/frontend.h>

#include "diseqc.hpp"

// Beschreibt einen Transponder.
struct SatelliteTune
{
    // Satellitensteuerung.
    diseqc_modes lnbMode;
    __u32 lnb1;
    __u32 lnb2;
    __u32 lnbSwitch;
    bool lnbPower;
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
    add_section_filter = 0,
    add_stream_filter = 1,
    connect_adapter = 2,
    del_all_filters = 3,
    del_filter = 4,
    tune = 5,
};

// Bezeichnet die gewünschte Hardware (frontend_request::connect_adapter).
struct connect_request
{
    int adapter;
    int frontend;
};

// Meldung an den Client.
struct response
{
    // Größe der folgenden Nutzdaten in Bytes.
    int len;
    // Nutzdaten.
    char payload[0];
};

#endif