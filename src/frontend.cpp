#include "frontend.hpp"
#include "sectionFilter.hpp"
#include "streamFilter.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

void Frontend::close()
{
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

void Frontend::readStatus()
{
    for (;; ::sleep(5))
    {
        if (::ioctl(_fd, FE_READ_STATUS, &_signal.status) != 0)
        {
            break;
        }

        if (::ioctl(_fd, FE_READ_SIGNAL_STRENGTH, &_signal.strength) != 0)
        {
            break;
        }

        if (::ioctl(_fd, FE_READ_SNR, &_signal.snr) != 0)
        {
            break;
        }

        if (::ioctl(_fd, FE_READ_BER, &_signal.ber) != 0)
        {
            break;
        }
    }
}

bool Frontend::open()
{
    close();

    char path[40];

    ::sprintf(path, "/dev/dvb/adapter%i/frontend%i", adapter, frontend);

    _fd = ::open(path, O_RDWR);

    if (_fd < 0)
    {
        return false;
    }

    _status = new std::thread(&Frontend::readStatus, this);

    return true;
}

bool Frontend::tune(const SatelliteTune &transponder)
{
    if (!isOpen())
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
        return false;
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

    return (::ioctl(_fd, FE_SET_PROPERTY, &dtv_prop) == 0);
}

void Frontend::createSectionFilter(__u16 pid)
{
    removeFilter(pid);

    _filters[pid] = new SectionFilter(*this, pid);
}

void Frontend::createStreamFilter(__u16 pid)
{
    removeFilter(pid);

    _filters[pid] = new StreamFilter(*this, pid);
}

bool Frontend::startFilter(__u16 pid)
{
    auto filter = _filters.find(pid);

    if (filter == _filters.end())
    {
        return false;
    }

    filter->second->start();

    return true;
}

bool Frontend::removeFilter(__u16 pid)
{
    auto filter = _filters.find(pid);

    if (filter == _filters.end())
    {
        return false;
    }

    _filters.erase(pid);

    delete filter->second;

    return true;
}

void Frontend::removeAllFilters()
{
    for (auto &filter : _filters)
    {
        delete filter.second;
    }

    _filters.clear();
}