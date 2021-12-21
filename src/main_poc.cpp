extern "C"
{
#include <libdvbv5/dvb-fe.h>
#include <libdvbv5/dvb-demux.h>
}

#include <stdio.h>

#include "manager.hpp"

int main2()
{
    auto fe = dvb_fe_open(0, 0, 0, 0);

    if (!fe)
    {
        return errno;
    }

    SatelliteTune transponder = {
        .lnbMode = diseqc_modes::diseqc1,
        .modulation = QPSK,
        .frequency = 12187500,
        .symbolrate = 27500000,
        .horizontal = true,
        .innerFEC = FEC_3_4,
        .s2 = false,
        .rolloff = ROLLOFF_AUTO,
    };

    auto lnbIndex = dvb_sat_search_lnb("EXTENDED");

    if (lnbIndex < 0)
    {
        printf("LNB: %d\n", errno);
    }

    fe->lnb = dvb_sat_get_lnb(lnbIndex);

    dvb_fe_store_parm(fe, DTV_DELIVERY_SYSTEM, transponder.s2 ? fe_delivery_system::SYS_DVBS2 : fe_delivery_system::SYS_DVBS);
    dvb_fe_store_parm(fe, DTV_FREQUENCY, transponder.frequency);
    dvb_fe_store_parm(fe, DTV_MODULATION, transponder.modulation);
    dvb_fe_store_parm(fe, DTV_SYMBOL_RATE, transponder.symbolrate);
    dvb_fe_store_parm(fe, DTV_INNER_FEC, transponder.innerFEC);
    dvb_fe_store_parm(fe, DTV_ROLLOFF, transponder.rolloff);

    if (dvb_fe_set_parms(fe))
    {
        printf("PARAMS: %d\n", errno);
    }

    auto data = dvb_dmx_open(0, 0);

    if (!data)
    {
        printf("DEMUX: %d\n", errno);
    }

    if (dvb_set_pesfilter(data, 0x2000, DMX_PES_OTHER, DMX_OUT_TAP, 10 * 1024 * 1024))
    {
        printf("FILTER: %d\n", errno);
    }

    auto buffer = new uint8_t[100000];
    auto total = 0;

    auto wr = ::open("dump.bin", O_CREAT | O_TRUNC | O_WRONLY, 0x777);

    for (; total < 100000000;)
    {
        auto bytes = read(data, buffer, 100000);

        if (!bytes)
        {
            break;
        }

        if (bytes < 0)
        {
            if (errno != EAGAIN)
            {
                printf("READ: %d\n", errno);

                break;
            }

            continue;
        }

        ::write(wr, buffer, bytes);

        total += bytes;
    }

    delete buffer;

    ::close(wr);

    printf("TOTAL: %d\n", total);

    dvb_dmx_close(data);
    dvb_fe_close(fe);
}
