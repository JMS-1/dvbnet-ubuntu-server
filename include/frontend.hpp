#ifndef _DVBNET_FRONTEND_H
#define _DVBNET_FRONTEND_H 1

#include <linux/dvb/frontend.h>

#include <map>
#include <thread>

#include "diseqc.hpp"

class Filter;

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

enum frontend_response
{
    signal = 0,
    section = 1,
    stream = 2,
};

class FrontendManager;

class Frontend
{
    friend class Filter;

public:
    Frontend(int adapter, int frontend, FrontendManager *manager)
        : _fd(-1),
          _manager(manager),
          _status(nullptr),
          adapter(adapter),
          frontend(frontend)
    {
    }

    ~Frontend();

private:
    FrontendManager *_manager;
    std::map<__u16, Filter *> _filters;
    std::thread *_status;
    int _fd;

private:
    void readStatus();
    void sendResponse(frontend_response type, __u16 pid, const void *payload, int payloadSize);

public:
    const int adapter;
    const int frontend;

public:
    const bool isOpen() { return _fd >= 0; }

public:
    bool open();
    bool tune(const SatelliteTune &transponder);
    void close();

public:
    void createSectionFilter(__u16 pid);
    void createStreamFilter(__u16 pid);
    bool startFilter(__u16 pid);
    bool removeFilter(__u16 pid);
    void removeAllFilters();
};

#endif