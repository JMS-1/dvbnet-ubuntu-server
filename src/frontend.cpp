#include "frontend.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cerrno>
#include <thread>

enum diseqc_modes
{
    burst_off,
    burst_on,
    diseqc1,
    diseqc2,
    diseqc3,
    diseqc4,
    none,
};

class DiSEqCMessage
{
public:
    DiSEqCMessage(__u8 message[], __u8 burst, __u8 repeat) : message(message), burst(burst), repeat(repeat) {}

public:
    const __u8 *message;
    const __u8 burst;
    const __u8 repeat;

public:
    int send(int fd)
    {
        if (message == nullptr)
        {
            return ::ioctl(fd, FE_DISEQC_SEND_BURST, burst ? fe_sec_mini_cmd::SEC_MINI_B : fe_sec_mini_cmd::SEC_MINI_A);
        }

        dvb_diseqc_master_cmd cmd = {0};

        cmd.msg_len = (repeat == 1) ? 3 : 4;

        ::memcpy(cmd.msg, message, cmd.msg_len);

        return ::ioctl(fd, FE_DISEQC_SEND_MASTER_CMD, &cmd);
    }

public:
    static DiSEqCMessage create(diseqc_modes mode, bool highFrequency, bool horizontal)
    {
        __u8 choice = 0;

        if (highFrequency)
            choice |= 0x01;
        if (horizontal)
            choice |= 0x02;

        switch (mode)
        {
        case diseqc_modes::none:
            return DiSEqCMessage(new __u8[3]{0xe0, 0x00, 0x00}, 0xff, 1);
        case diseqc_modes::burst_on:
            return DiSEqCMessage(nullptr, 0, 1);
        case diseqc_modes::burst_off:
            return DiSEqCMessage(nullptr, 1, 1);
        case diseqc_modes::diseqc1:
            return DiSEqCMessage(new __u8[4]{0xe0, 0x10, 0x38, (__u8)(0xf0 | choice)}, 0xff, 3);
        case diseqc_modes::diseqc2:
            return DiSEqCMessage(new __u8[4]{0xe0, 0x10, 0x38, (__u8)(0xf4 | choice)}, 0xff, 3);
        case diseqc_modes::diseqc3:
            return DiSEqCMessage(new __u8[4]{0xe0, 0x10, 0x38, (__u8)(0xf8 | choice)}, 0xff, 3);
        case diseqc_modes::diseqc4:
            return DiSEqCMessage(new __u8[4]{0xe0, 0x10, 0x38, (__u8)(0xfc | choice)}, 0xff, 3);
        default:
            throw "unsupported DiSEqC mode";
        }
    }
};

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

    _fd = ::open("/dev/dvb/adapter0/frontend0", O_RDWR);

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

void SectionFilter::stop()
{
    if (_fd < 0)
    {
        return;
    }

    close(_fd);

    _fd = -1;
}

void onThread(int fd)
{
    auto dump = ::open("dump.epg", O_WRONLY | O_CREAT | O_TRUNC);

    ssize_t total = 0;

    __u8 buffer[1000];

    while (total < 2000000)
    {
        auto bytes = ::read(fd, buffer, sizeof(buffer));

        if (bytes <= 0)
        {
            throw "no EPG data";
        }

        ::write(dump, buffer, bytes);

        total += bytes;
    }

    ::close(dump);

    ::printf("total %ld\n", total);
}

void SectionFilter::start()
{
    stop();

    _fd = ::open("/dev/dvb/adapter0/demux0", O_RDWR);

    if (_fd < 0)
    {
        throw "unable to create filter";
    }

    dmx_sct_filter_params filter = {
        .pid = _pid,
        .filter = {0},
        .timeout = 0,
        .flags = DMX_IMMEDIATE_START | DMX_CHECK_CRC};

    auto pid_error = ::ioctl(_fd, DMX_SET_FILTER, &filter);

    if (pid_error != 0)
    {
        throw "unable to register filter";
    }

    std::thread reader(onThread, _fd);

    reader.join();
}