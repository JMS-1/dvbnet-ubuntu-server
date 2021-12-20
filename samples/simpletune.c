/*
 * (c) 2008 Dominik Kuhlen
 * Simple DVB tuner which supports: DVB-S DVB-S2
 * it does only tune nothing more.
 *
 */
#include <stdio.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>

/* LinuxTV includes must be from HG */
#include <linux/dvb/frontend.h>

#ifndef DVBFE_SET_DELSYS
#error you are trying to includ wrong linuxtv headers
#endif

/* wait __ usec after ioctl calls */
#define TUNE_DELAY 50000

#define HOR 1
#define VER 0

#define DELSYS_DVBS 1
#define DELSYS_DVBS2 2

int do_mini_diseqc(int fd, int satno)
{
    fe_sec_mini_cmd_t cmd = SEC_MINI_A;
    if (satno == 1)
    {
        cmd = SEC_MINI_B;
    }
    if (ioctl(fd, FE_DISEQC_SEND_BURST, cmd) != 0)
    {
        perror("SEND BURST");
        return -1;
    }
    return 0;
}

int do_diseqc(int fd, int satno, fe_sec_voltage_t voltage, int tone)
{
    struct dvb_diseqc_master_cmd cmd;
    cmd.msg[0] = 0xe0;
    cmd.msg[1] = 0x10;
    cmd.msg[2] = 0x38;
    cmd.msg[3] = 0xf0 | ((satno * 4) & 0x0F) |
                 (tone == SEC_TONE_ON ? 1 : 0) |
                 (voltage == SEC_VOLTAGE_18 ? 2 : 0);
    cmd.msg_len = 4;
    if (ioctl(fd, FE_DISEQC_SEND_MASTER_CMD, &cmd) < 0)
    {
        perror("DISEQC SEND: ");
        return -1;
    }
    return 0;
}

/* tune qpsk */
/* freq: khz: frequency of transponder */
/* vpid, apid, tpid: PIDs of video, audio and teletext TS packets */
/* satno: DiSEqC satellite number */
/* pol: 1: hor, 0: ver Polarisation */
/* srate: kSym/s Symbol Rate */
/* fec. FEC */
/* lnb_lof1: local frequency of lower LNB band kHz */
/* lnb_lof2: local frequency of upper LNB band kHz */
/* lnb_slof: switch frequency of LNB kHz */
int set_qpsk_channel(int front, int mydelsys, int freq, int satno, int pol, int srate, int lnb_lof1, int lnb_lof2, int lnb_slof)
{
    struct dvb_frontend_parameters FEP;
    fe_status_t stat;
    int tone;
    fe_sec_voltage_t voltage;
    struct dvbfe_params feparams;
    enum dvbfe_delsys delsys;
    int flo; /* 9750 / 10600 */

    switch (mydelsys)
    {
    case DELSYS_DVBS:
        delsys = DVBFE_DELSYS_DVBS;
        break;
    case DELSYS_DVBS2:
        delsys = DVBFE_DELSYS_DVBS2;
        break;
    default:
        fprintf(stderr, "unknown delsys: %d\n", delsys);
        return -1;
    }
    if (ioctl(front, DVBFE_SET_DELSYS, &delsys) < 0)
    {
        perror("DVBFE_SET_DELSYS");
    }

    /* power off only */
    if (freq == 0)
    {
        voltage = SEC_VOLTAGE_OFF;
        printf("ioclt: FE_SET_VOLTAGE : %d (off)\n", voltage);
        if (ioctl(front, FE_SET_VOLTAGE, voltage) < 0)
        {
            perror("FE VOLTAGE: ");
            return -1;
        }
        return 0;
    }

    if (ioctl(front, FE_SET_TONE, SEC_TONE_OFF) != 0)
    {
        perror("FE TONE: ");
        return -1;
    }
    usleep(TUNE_DELAY);

    if (pol == HOR)
        voltage = SEC_VOLTAGE_18;
    else
        voltage = SEC_VOLTAGE_13;
    printf("ioclt: FE_SET_VOLTAGE : %d\n", voltage);
    if (ioctl(front, FE_SET_VOLTAGE, voltage) < 0)
    {
        perror("FE VOLTAGE: ");
        return -1;
    }

    usleep(TUNE_DELAY);

    if (freq < lnb_slof)
    {
        printf("Low band\n");
        FEP.frequency = (freq - lnb_lof1);
        flo = lnb_lof1;
        tone = SEC_TONE_OFF;
    }
    else
    {
        printf("High band\n");
        FEP.frequency = (freq - lnb_lof2);
        flo = lnb_lof2;
        tone = SEC_TONE_ON;
    }

#ifndef _NO_DISEQC_
    do_diseqc(front, satno, voltage, tone);
#endif

    printf("tone: %d\n", tone == SEC_TONE_ON);
    if (ioctl(front, FE_SET_TONE, tone) != 0)
    {
        perror("FE TONE: ");
        return -1;
    }

    memset(&feparams, 0, sizeof(feparams));
    feparams.frequency = FEP.frequency;
    feparams.inversion = DVBFE_INVERSION_AUTO;

    switch (delsys)
    {
    case DVBFE_DELSYS_DVBS2:
        feparams.delivery = DVBFE_DELSYS_DVBS2;
        feparams.delsys.dvbs2.symbol_rate = srate;
        feparams.delsys.dvbs2.modulation = DVBFE_MOD_AUTO;
        feparams.delsys.dvbs2.fec = DVBFE_FEC_AUTO;
        printf("dvbfe setparams :  delsys=%d %dMHz / Rate : %dkBPS\n", feparams.delivery, feparams.frequency / 1000,
               feparams.delsys.dvbs2.symbol_rate / 1000);
        break;
    case DVBFE_DELSYS_DVBS:
        feparams.delivery = DVBFE_DELSYS_DVBS;
        feparams.delsys.dvbs.symbol_rate = srate;
        feparams.delsys.dvbs.modulation = DVBFE_MOD_AUTO;
        feparams.delsys.dvbs.fec = DVBFE_FEC_AUTO;
        printf("dvbfe setparams :  delsys=%d %dMHz / Rate : %dkBPS\n", feparams.delivery, feparams.frequency / 1000,
               feparams.delsys.dvbs.symbol_rate / 1000);
        break;
    default:
        return -1;
    }

    if (ioctl(front, DVBFE_SET_PARAMS, &feparams) < 0)
    {
        perror("DVBFE_SET_PARAMS");
        return -1;
    }
    usleep(TUNE_DELAY);

    ioctl(front, FE_READ_STATUS, &stat);
    return stat;
}

char *FStatNames[7] = {"\033[1;36mSignal\033[0m", "\033[1;32mCarrier\033[0m", "\033[1;33mViterbi\033[0m", "\033[1;34mSync\033[0m", "\033[1;35mLock\033[0m", "Timeout", "Reinit"};

void printHelp()
{
    printf("-f [freq]     MHz               (default: 12266)\n");
    printf("-p [polaris]  h|v               (default: h)\n");
    printf("-s [symrate]  kSym/s            (default: 27500)\n");
    printf("-n [satnum]   in DiSEqC message (default: 0)\n");
    printf("-d [delivery] 1/2 (->DVB-S1/S2) (default: 1)\n");
    printf("-a [adapter]  adapter number    (deafult: 0)\n");
}

int main(int argc, char **argv)
{
    struct dvb_frontend_info info;
    struct dvbfe_params feparams;
    int delsys = DELSYS_DVBS;
    int freq = 12266;
    int pol = HOR;
    int srate = 27500;
    int sat = 0;
    int c, ret;
    uint16_t snr, rssi;
    uint32_t ber;
    int fdfe;
    fe_status_t stat;
    int adapter = 0;
    char fedev[64];

    while ((c = getopt(argc, argv, "a:f:p:s:d:n:h")) != -1)
    {
        switch (c)
        {
        case 'a':
            adapter = atoi(optarg);
            break;
        case 'f':
            freq = atof(optarg);
            break;
        case 'p':
            pol = (optarg[0] | 0x20) == 'h' ? HOR : VER;
            break;
        case 's':
            srate = atoi(optarg);
            break;
        case 'd':
            delsys = atoi(optarg);
            break;
        case 'n':
            sat = atoi(optarg);
            break;
        case 'h':
        default:
            printHelp();
            return 0;
        }
    }

    snprintf(fedev, sizeof(fedev), "/dev/dvb/adapter%d/frontend0", adapter);

    printf("using '%s' as frontend\n", fedev);

    fdfe = open(fedev, O_RDWR);
    if (fdfe < 0)
    {
        perror("Frontend");
        return 1;
    }

    ret = ioctl(fdfe, FE_GET_INFO, &info);
    if (ret < 0)
    {
        perror("FE_GET_INFO");
    }
    printf("frontend fd=%d: type=%d\n", fdfe, info.type);

    switch (info.type)
    {
    case FE_QPSK:
        if (set_qpsk_channel(fdfe, delsys, freq * 1000, sat, pol, srate * 1000, 9750000, 10600000, 11700000) < 0)
        {
            printf("tuning qpsk failed\n");
            return 1;
        }
        break;
    default:
        printf("unsupported frontend: %d\n", info.type);
        return 0;
    }

    c = 0;
    do
    {
        stat = FE_TIMEDOUT;
        if (ioctl(fdfe, FE_READ_STATUS, &stat) < 0)
        {
            perror("FE_READ_STATUS");
        }
        else
        {
            printf("Status: %02x: ", stat);
            for (c = 0; c < 7; c++)
            {
                if (stat & (1 << c))
                    printf("%s ", FStatNames[c]);
            }
            printf("\n");
        }
        snr = 0xdead;
        if (ioctl(fdfe, FE_READ_SNR, &snr) < 0)
        {
            perror("FE_READ_SNR");
        }
        else
        {
            printf("SNR: %d %d (0x%x) (%.1fdB)\n", (snr >> 8) & 0xff, snr & 0xff, snr, snr * 0.1);
        }
        ber = 0xdead;
        if (ioctl(fdfe, FE_READ_BER, &ber) < 0)
        {
            perror("FE_READ_BER");
        }
        else
        {
            printf("BER: %d %d %d %d (0x%x)\n", (ber >> 24) & 0xff, (ber >> 16) & 0xff, (ber >> 8) & 0xff, ber & 0xff, ber);
        }
        rssi = 0xdead;
        if (ioctl(fdfe, FE_READ_SIGNAL_STRENGTH, &rssi) < 0)
        {
            perror("FE_READ_SIGNAL_STRENGTH");
        }
        else
        {
            printf("Signal: %d %d (0x%x) %d (%.1fdBm)\n", (rssi >> 8) & 0xff, rssi & 0xff, rssi, (int16_t)rssi, rssi * 0.1);
        }
        if (ioctl(fdfe, DVBFE_GET_PARAMS, &feparams) != 0)
        {
            perror("DVBFE_GET_PARAMS");
        }
        else
        {
            printf("Frontend: f=%.3f\n", feparams.frequency / 1000.0);
        }
        /* wait for <enter> or <CTRL-d> */
        ret = read(0, &c, 1);
    } while (ret > 0);

    /* power off */
    set_qpsk_channel(fdfe, delsys, 0, 0, pol, 0, 9750000, 10600000, 11700000);

    close(fdfe);
    return 0;
}
