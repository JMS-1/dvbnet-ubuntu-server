#include "diseqc.hpp"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/dvb/frontend.h>
#include <sys/ioctl.h>

/*
    Übermittelt einen DiSEqC Steuerbefehl an ein Frontend.
*/
int DiSEqCMessage::send(int fd)
{
    // Burst-Mode Umschaltung.
    if (message == nullptr)
    {
        return ::ioctl(fd, FE_DISEQC_SEND_BURST, burst ? fe_sec_mini_cmd::SEC_MINI_B : fe_sec_mini_cmd::SEC_MINI_A);
    }

    // Standardumschaltung.
    dvb_diseqc_master_cmd cmd = {{0}, static_cast<__u8>((repeat == 1) ? 3 : 4)};

    ::memcpy(cmd.msg, message, cmd.msg_len);

    auto err = ::ioctl(fd, FE_DISEQC_SEND_MASTER_CMD, &cmd);

#ifdef DEBUG
    // Protokollierung.
    if (err != 0)
    {
        ::printf("DiSEqC error %d (%d)\n", err, errno);
    }
#endif
}

/*
    Erstellt die Beschreibung für einen DiSEqC Befehl.
*/
DiSEqCMessage DiSEqCMessage::create(diseqc_modes mode, bool highFrequency, bool horizontal)
{
    // Vorauswahl für die Standardumschaltung.
    __u8 choice = 0;

    if (highFrequency)
        choice |= 0x01;
    if (horizontal)
        choice |= 0x02;

    // Nachrichten je nach Art der DiSEqC Steuerung (der Einfachheit halber aus DVB.NET übernommen).
    switch (mode)
    {
    case diseqc_modes::none:
        return DiSEqCMessage(new __u8[3]{0xe0, 0x00, 0x00}, 1);
    case diseqc_modes::burst_on:
        return DiSEqCMessage(true);
    case diseqc_modes::burst_off:
        return DiSEqCMessage(false);
    case diseqc_modes::diseqc1:
        return DiSEqCMessage(new __u8[4]{0xe0, 0x10, 0x38, static_cast<__u8>(0xf0 | choice)}, 3);
    case diseqc_modes::diseqc2:
        return DiSEqCMessage(new __u8[4]{0xe0, 0x10, 0x38, static_cast<__u8>(0xf4 | choice)}, 3);
    case diseqc_modes::diseqc3:
        return DiSEqCMessage(new __u8[4]{0xe0, 0x10, 0x38, static_cast<__u8>(0xf8 | choice)}, 3);
    case diseqc_modes::diseqc4:
        return DiSEqCMessage(new __u8[4]{0xe0, 0x10, 0x38, static_cast<__u8>(0xfc | choice)}, 3);
    default:
        throw "unsupported DiSEqC mode";
    }
}
