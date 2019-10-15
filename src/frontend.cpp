#include "frontend.hpp"

#include "manager.hpp"
#include "sectionFilter.hpp"
#include "streamFilter.hpp"

#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

Frontend::Frontend(int tcp, FrontendManager *manager)
    : _active(true),
      _fd(-1),
      _manager(manager),
      _status(nullptr),
      _tcp(tcp),
      adapter(adapter),
      frontend(frontend)
{
    _listener = new std::thread(&Frontend::waitRequest, this);
}

Frontend::~Frontend()
{
    _active = false;

    close();

    auto manager = _manager;

    if (!manager)
    {
        return;
    }

    _manager = nullptr;

    manager->removeFrontend(adapter, frontend);
}

bool readblock(int fd, void *buffer, int len)
{
    auto cbuffer = static_cast<char *>(buffer);

    while (len > 0)
    {
        auto read = ::read(fd, cbuffer, len);

        if (read <= 0)
        {
            return false;
        }

        cbuffer += read;
        len -= read;
    }

    return true;
}

bool Frontend::processConnect()
{
    connect_request request;

    if (!readblock(_tcp, &request, sizeof(request)))
    {
        return false;
    }

    if (!_active)
    {
        return false;
    }

    char path[40];

    ::sprintf(path, "/dev/dvb/adapter%i/frontend%i", adapter, frontend);

    _fd = ::open(path, O_RDWR);

    if (_fd < 0)
    {
        return false;
    }

    _status = new std::thread(&Frontend::readStatus, this);

    auto manager = _manager;

    if (!manager)
    {
        return false;
    }

    return manager->addFrontend(this);
}

bool Frontend::processTune()
{
    SatelliteTune transponder;

    if (!readblock(_tcp, &transponder, sizeof(transponder)))
    {
        return false;
    }

    if (!_active)
    {
        return false;
    }

    if (_fd < 0)
    {
        return false;
    }

    removeAllFilters();

    auto useSwitch = (transponder.lnbMode >= diseqc_modes::diseqc1) && (transponder.lnbMode <= diseqc_modes::diseqc4);
    auto hiFreq = useSwitch && transponder.frequency >= transponder.lnbSwitch;
    auto freq = transponder.frequency - (hiFreq ? transponder.lnb2 : 0);

    DiSEqCMessage diseqc(DiSEqCMessage::create(diseqc_modes::diseqc1, hiFreq, transponder.horizontal));

    if (diseqc.send(_fd) != 0)
    {
        return true;
    }

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

    ::ioctl(_fd, FE_SET_PROPERTY, &dtv_prop);

    return true;
}

bool Frontend::processAddSection()
{
    __u16 pid;

    if (!readblock(_tcp, &pid, sizeof(pid)))
    {
        return false;
    }

    if (!_active)
    {
        return false;
    }

    removeFilter(pid);

    auto filter = new SectionFilter(*this, pid);
#include <unistd.h>

    _filters[pid] = filter;

    filter->start();

    return true;
}

bool Frontend::processAddStream()
{
    __u16 pid;

    if (!readblock(_tcp, &pid, sizeof(pid)))
    {
        return false;
    }

    if (!_active)
    {
        return false;
    }

    removeFilter(pid);

    auto filter = new StreamFilter(*this, pid);

    _filters[pid] = filter;

    filter->start();

    return true;
}

bool Frontend::processRemoveFilter()
{
    __u16 pid;

    if (!readblock(_tcp, &pid, sizeof(pid)))
    {
        return false;
    }

    if (!_active)
    {
        return false;
    }

    removeFilter(pid);

    return true;
}

bool Frontend::processRemoveAllFilters()
{
    removeAllFilters();

    return true;
}

void Frontend::waitRequest()
{
    for (;;)
    {
        frontend_request request;

        if (!readblock(_tcp, &request, sizeof(request)))
        {
            break;
        }

        auto ok = false;

        switch (request)
        {
        case frontend_request::connect_adapter:
            ok = processConnect();
            break;
        case frontend_request::tune:
            ok = processTune();
            break;
        case frontend_request::add_section_filter:
            ok = processAddSection();
            break;
        case frontend_request::add_stream_filter:
            ok = processAddStream();
            break;
        case frontend_request::del_filter:
            ok = processRemoveFilter();
            break;
        case frontend_request::del_all_filters:
            ok = processRemoveAllFilters();
            break;
        }

        if (!ok)
        {
            break;
        }
    }

    if (_active)
    {
        close(true);
    }
}

void Frontend::close()
{
    close(false);
}

void Frontend::close(bool nowait)
{
    auto tcp = _tcp;

    if (tcp < 0)
    {
        return;
    }

    _tcp = -1;

    ::close(tcp);

    auto listener = _listener;

    if (!listener)
    {
        return;
    }

    _listener = nullptr;

    if (listener->joinable() && !nowait)
    {
        listener->join();
    }

    auto fd = _fd;

    if (fd < 0)
    {
        return;
    }

    _fd = -1;

    removeAllFilters();

    ::close(fd);

    auto status = _status;

    if (!status)
    {
        return;
    }

    _status = nullptr;

    if (status->joinable())
    {
        status->join();
    }
}

void Frontend::sendResponse(frontend_response type, __u16 pid, const void *payload, int payloadSize)
{
    struct response
    {
        frontend_response type;
        __u16 pid;
        int len;
        char payload[0];
    };

    auto total = sizeof(response) + payloadSize;

    response *data = reinterpret_cast<response *>(::malloc(total));

    data->type = type;
    data->pid = pid;
    data->len = payloadSize;

    ::memcpy(data->payload, payload, payloadSize);

    ::write(_tcp, data, total);
}

void Frontend::readStatus()
{
    for (;; ::sleep(5))
    {
        signal_response signal = {0};

        if (::ioctl(_fd, FE_READ_STATUS, &signal.status) != 0)
        {
            break;
        }

        if (::ioctl(_fd, FE_READ_SIGNAL_STRENGTH, &signal.strength) != 0)
        {
            break;
        }

        if (::ioctl(_fd, FE_READ_SNR, &signal.snr) != 0)
        {
            break;
        }

        if (::ioctl(_fd, FE_READ_BER, &signal.ber) != 0)
        {
            break;
        }

        sendResponse(frontend_response::signal, 0, &signal, sizeof(signal));
    }
}

void Frontend::removeFilter(__u16 pid)
{
    auto filter = _filters.find(pid);

    if (filter == _filters.end())
    {
        return;
    }

    _filters.erase(pid);

    delete filter->second;
}

void Frontend::removeAllFilters()
{
    for (auto &filter : _filters)
    {
        delete filter.second;
    }

    _filters.clear();
}