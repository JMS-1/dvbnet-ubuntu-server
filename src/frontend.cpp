#include "frontend.hpp"
#include "diseqc.hpp"
#include "sectionFilter.hpp"
#include "streamFilter.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

bool Frontend::close()
{
    if (_fd < 0)
    {
        return false;
    }

    ::close(_fd);

    _fd = -1;

    return true;
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

    dvb_frontend_info info;

    ::ioctl(_fd, FE_GET_INFO, &info);

    return true;
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

int Frontend::tune()
{
    if (!isOpen())
    {
        throw "frontend not open";
    }

    DiSEqCMessage diseqc(DiSEqCMessage::create(diseqc_modes::diseqc1, true, true));

    auto diseqc_err = diseqc.send(_fd);

    if (diseqc_err != 0)
    {
        return diseqc_err;
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

    auto tune_error = ::ioctl(_fd, FE_SET_PROPERTY, &dtv_prop);

    if (tune_error != 0)
    {
        printf("%d\n", errno);

        throw "unable to tune";
    }

    return 0;
}

Filter &Frontend::createSectionFilter(__u16 pid)
{
    return *new SectionFilter(*this, pid);
}

Filter &Frontend::createStreamFilter(__u16 pid)
{
    return *new StreamFilter(*this, pid);
}
