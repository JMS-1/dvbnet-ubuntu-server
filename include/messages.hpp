#ifndef _DVBNET_MESSAGES_H
#define _DVBNET_MESSAGES_H 1

#include <linux/dvb/frontend.h>

#include "diseqc.hpp"

struct SatelliteTune
{
    diseqc_modes lnbMode;
    __u32 lnb1;
    __u32 lnb2;
    __u32 lnbSwitch;
    bool lnbPower;

    fe_modulation modulation;
    __u32 frequency;
    __u32 symbolrate;
    bool horizontal;
    fe_code_rate innerFEC;
    bool s2;
    fe_rolloff rolloff;
};

enum frontend_request
{
    add_section_filter = 0,
    add_stream_filter = 1,
    connect_adapter = 2,
    del_all_filters = 3,
    del_filter = 4,
    tune = 5,
};

struct connect_request
{
    int adapter;
    int frontend;
};

enum frontend_response
{
    section = 0,
    signal = 1,
    stream = 2,
};

struct signal_response
{
    __u16 snr;
    __u16 strength;
    __u32 ber;
    fe_status_t status;
};

struct response
{
    frontend_response type;
    __u16 pid;
    int len;
    char payload[0];
};

#endif