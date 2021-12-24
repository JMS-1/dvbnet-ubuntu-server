#include "diseqc.hpp"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <linux/dvb/frontend.h>
#include <sys/ioctl.h>
#include <unistd.h>

class DiSEqCMessageS23600 : public DiSEqCMessage
{
public:
    // Erzeugt eine Burst-Mode Steuermeldung.
    DiSEqCMessageS23600(diseqc_modes mode, bool highFrequency, bool horizontal) : _mode(mode), _highFrequency(highFrequency), _horizontal(horizontal) {}

private:
    // Auswahl des Satelliten.
    const diseqc_modes _mode;
    // Gesetzt wenn der oberere Frequenzbereich verwendet werden soll.
    const bool _highFrequency;
    // Gesetzt bei einer horizontalen Polarisierung.
    const bool _horizontal;

public:
    void _send(int fd)
    {
        // Schaltung vorbereiten.
        ioctl(fd, FE_SET_TONE, SEC_TONE_OFF);
        usleep(15000);

        // Auswahl der Polarisierung.
        ioctl(fd, FE_SET_VOLTAGE, _horizontal ? SEC_VOLTAGE_18 : SEC_VOLTAGE_13);
        usleep(15000);

        // Auswahl des Satelliten.
        ioctl(fd, FE_DISEQC_SEND_BURST, _mode == diseqc_modes::diseqc2 || _mode == diseqc_modes::diseqc4 ? SEC_MINI_B : SEC_MINI_A);
        usleep(15000);

        // Auswahl des Frequenzbereichs.
        ioctl(fd, FE_SET_TONE, _highFrequency ? SEC_TONE_ON : SEC_TONE_OFF);
        usleep(15000);
    }
};

/*
    Übermittelt einen DiSEqC Steuerbefehl an ein Frontend.
*/
void DiSEqCMessage::_send(int fd)
{
    int err;

    if (repeat)
    {
        // Standardumschaltung.
        dvb_diseqc_master_cmd cmd = {{0}, static_cast<__u8>((repeat == 1) ? 3 : 4)};

        memcpy(cmd.msg, _message, cmd.msg_len);

        err = ioctl(fd, FE_DISEQC_SEND_MASTER_CMD, &cmd);
    }
    else
    {
        // Burst-Mode Umschaltung.
        err = ioctl(fd, FE_DISEQC_SEND_BURST, burst ? fe_sec_mini_cmd::SEC_MINI_B : fe_sec_mini_cmd::SEC_MINI_A);
    }

#ifdef DEBUG
    // Protokollierung.
    if (err)
        printf("DiSEqC error %d (%d)\n", err, errno);
#endif
}

/*
    Erstellt die Beschreibung für einen DiSEqC Befehl.
*/
void DiSEqCMessage::send(diseqc_modes mode, bool highFrequency, bool horizontal, int fe)
{
    DiSEqCMessage *diseqc = nullptr;

    // Geräte für Sonderbehandlungen prüfen.
    if (mode >= diseqc_modes::diseqc1 && mode <= diseqc_modes::diseqc4)
    {
        dvb_frontend_info info = {0};

        if (!::ioctl(fe, FE_GET_INFO, &info) && !strcmp(info.name, "STB0899 Multistandard"))
            diseqc = new DiSEqCMessageS23600(mode, highFrequency, horizontal);
    }

    // Standardgeräte.
    if (diseqc == nullptr)
    {
        // Vorauswahl für die Standardumschaltung.
        u_char choice = 0;

        if (highFrequency)
            choice |= 0x01;
        if (horizontal)
            choice |= 0x02;

        // Nachrichten je nach Art der DiSEqC Steuerung (der Einfachheit halber aus DVB.NET übernommen).
        switch (mode)
        {
        case diseqc_modes::none:
            diseqc = new DiSEqCMessage(1, 0xe0, 0x00, 0x00);
            break;
        case diseqc_modes::burst_on:
            diseqc = new DiSEqCMessage(true);
            break;
        case diseqc_modes::burst_off:
            diseqc = new DiSEqCMessage(false);
        case diseqc_modes::diseqc1:
            diseqc = new DiSEqCMessage(3, 0xe0, 0x10, 0x38, static_cast<__u8>(0xf0 | choice));
            break;
        case diseqc_modes::diseqc2:
            diseqc = new DiSEqCMessage(3, 0xe0, 0x10, 0x38, static_cast<__u8>(0xf4 | choice));
            break;
        case diseqc_modes::diseqc3:
            diseqc = new DiSEqCMessage(3, 0xe0, 0x10, 0x38, static_cast<__u8>(0xf8 | choice));
            break;
        case diseqc_modes::diseqc4:
            diseqc = new DiSEqCMessage(3, 0xe0, 0x10, 0x38, static_cast<__u8>(0xfc | choice));
            break;
        default:
            return;
        }
    }

    // Befehl ausführen.
    diseqc->_send(fe);

    // Speicher freigeben.
    delete diseqc;
}
