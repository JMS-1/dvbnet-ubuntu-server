//#define DUMP_STRUCT_LAYOUT

#include "manager.hpp"

int main()
{
#ifdef DUMP_STRUCT_LAYOUT
    ::printf(
        "SatelliteTune\nlnbMode=%d\nlnb1=%d\nlnb2=%d\nlnbSwitch=%d\nlnbPower=%d\nmodulation=%d\nfrequency=%d\nsymbolrate=%d\nhorizontal=%d\ns2=%d\nrolloff=%d\ntotal=%d\n\n",
        offsetof(SatelliteTune, lnbMode),
        offsetof(SatelliteTune, lnb1),
        offsetof(SatelliteTune, lnb2),
        offsetof(SatelliteTune, lnbSwitch),
        offsetof(SatelliteTune, lnbPower),
        offsetof(SatelliteTune, modulation),
        offsetof(SatelliteTune, frequency),
        offsetof(SatelliteTune, symbolrate),
        offsetof(SatelliteTune, horizontal),
        offsetof(SatelliteTune, s2),
        offsetof(SatelliteTune, rolloff),
        sizeof(SatelliteTune));

    ::printf(
        "connect_request\nadapter=%d\nfrontend=%d\ntotal=%d\n\n",
        offsetof(connect_request, adapter),
        offsetof(connect_request, frontend),
        sizeof(connect_request));

    ::printf(
        "signal_response\nstatus=%d\nsnr=%d\nstrength=%d\nber=%d\ntotal=%d\n\n",
        offsetof(signal_response, status),
        offsetof(signal_response, snr),
        offsetof(signal_response, strength),
        offsetof(signal_response, ber),
        sizeof(signal_response));

    ::printf(
        "response\ntype=%d\npid=%d\nlen=%d\ntotal=%d\n\n",
        offsetof(response, type),
        offsetof(response, pid),
        offsetof(response, len),
        sizeof(response));

    auto test = 0x01020304;
    auto test_ptr = reinterpret_cast<char *>(&test);

    ::printf("%d %d %d %d\n", test_ptr[0], test_ptr[1], test_ptr[2], test_ptr[3]);
#endif

    FrontendManager manager;

    if (!manager.listen())
    {
        ::exit(1);
    }

    ::printf("listener started\n");

    manager.run();
}
