#ifndef _DVBNET_FRONTEND_H
#define _DVBNET_FRONTEND_H 1

#include <linux/dvb/frontend.h>

#include <map>

#include "diseqc.hpp"
#include "filter.hpp"

class Frontend;

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

struct Signal
{
    __u16 snr;
    __u16 strength;
    __u32 ber;
    fe_status_t status;
};

class Frontend
{
public:
    Frontend(int adapter, int frontend) : adapter(adapter), frontend(frontend), _fd(-1), _status(nullptr), _signal({0})
    {
    }

    ~Frontend()
    {
        close();
    }

private:
    std::map<__u16, Filter *> _filters;
    std::thread *_status;
    int _fd;
    Signal _signal;

private:
    void readStatus();

public:
    const int adapter;
    const int frontend;

public:
    const bool isOpen() { return _fd >= 0; }

public:
    bool open();
    bool tune(const SatelliteTune &transponder);
    const Signal &getSignal() const { return _signal; }
    void close();

public:
    void createSectionFilter(__u16 pid);
    void createStreamFilter(__u16 pid);
    bool startFilter(__u16 pid);
    bool removeFilter(__u16 pid);
    void removeAllFilters();
};

#endif