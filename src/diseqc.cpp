#include "diseqc.hpp"

#include <linux/dvb/frontend.h>
#include <string.h>
#include <sys/ioctl.h>

int DiSEqCMessage::send(int fd)
{
    if (message == nullptr)
    {
        return ::ioctl(fd, FE_DISEQC_SEND_BURST, burst ? fe_sec_mini_cmd::SEC_MINI_B : fe_sec_mini_cmd::SEC_MINI_A);
    }

    dvb_diseqc_master_cmd cmd = {{0}, static_cast<__u8>((repeat == 1) ? 3 : 4)};

    ::memcpy(cmd.msg, message, cmd.msg_len);

    return ::ioctl(fd, FE_DISEQC_SEND_MASTER_CMD, &cmd);
}

DiSEqCMessage DiSEqCMessage::create(diseqc_modes mode, bool highFrequency, bool horizontal)
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
        return DiSEqCMessage(new __u8[4]{0xe0, 0x10, 0x38, static_cast<__u8>(0xf0 | choice)}, 0xff, 3);
    case diseqc_modes::diseqc2:
        return DiSEqCMessage(new __u8[4]{0xe0, 0x10, 0x38, static_cast<__u8>(0xf4 | choice)}, 0xff, 3);
    case diseqc_modes::diseqc3:
        return DiSEqCMessage(new __u8[4]{0xe0, 0x10, 0x38, static_cast<__u8>(0xf8 | choice)}, 0xff, 3);
    case diseqc_modes::diseqc4:
        return DiSEqCMessage(new __u8[4]{0xe0, 0x10, 0x38, static_cast<__u8>(0xfc | choice)}, 0xff, 3);
    default:
        throw "unsupported DiSEqC mode";
    }
}
