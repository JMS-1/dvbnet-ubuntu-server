#include "frontend.hpp"
#include "diseqc.hpp"
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
}

bool Frontend::open()
{
    close();

    char path[40];

    ::sprintf(path, "/dev/dvb/adapter%i/frontend%i", adapter, frontend);

    _fd = ::open(path, O_RDWR);

    return _fd >= 0;
}

const fe_status Frontend::getStatus()
{
    if (!isOpen())
    {
        throw "frontend not open";
    }

    fe_status status;

    if (::ioctl(_fd, FE_READ_STATUS, &status) < 0)
    {
        throw "unable to access frontend";
    }

    return status;
}

bool Frontend::tune()
{
    if (!isOpen())
    {
        return false;
    }

    DiSEqCMessage diseqc(DiSEqCMessage::create(diseqc_modes::diseqc1, true, true));

    if (diseqc.send(_fd) != 0)
    {
        return false;
    }

    struct dtv_property props[] =
        {
            {DTV_DELIVERY_SYSTEM, {0}, fe_delivery_system::SYS_DVBS},
            {DTV_FREQUENCY, {0}, 12187500 - 10600000},
            {DTV_INNER_FEC, {0}, fe_code_rate::FEC_3_4},
            {DTV_MODULATION, {0}, fe_modulation::QPSK},
            {DTV_SYMBOL_RATE, {0}, 27500000},
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