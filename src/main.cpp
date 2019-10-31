//#define DUMP_STRUCT_LAYOUT
#define RUN_TEST

#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "manager.hpp"

int readBuffer(int fd, void *buf, int len)
{
    auto dest = static_cast<char *>(buf);

    for (auto rest = len; rest > 0;)
    {
        auto bytes = ::read(fd, dest, rest);

        if (bytes <= 0)
            return -1;

        dest += bytes;
        rest -= bytes;
    }

    return len;
}

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

#ifdef RUN_TEST
    auto server = ::gethostbyname("localhost");

    sockaddr_in addr = {.sin_family = AF_INET, .sin_port = ::htons(29713), {0}};

    ::memcpy(&addr.sin_addr.s_addr, server->h_addr, server->h_length);

    auto fd = socket(AF_INET, SOCK_STREAM, 0);
    auto err = ::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));

    if (err != 0)
    {
        printf("connect: %d %d\n", err, errno);
    }

    auto cr = frontend_request::connect_adapter;
    connect_request cr_data = {.adapter = 0, .frontend = 0};

    ::write(fd, &cr, sizeof(cr));
    ::write(fd, &cr_data, sizeof(cr_data));

    SatelliteTune rtlplus = {
        .lnbMode = diseqc_modes::diseqc1,
        .lnb1 = 9750000,
        .lnb2 = 10600000,
        .lnbSwitch = 11700000,
        .lnbPower = true,
        .modulation = fe_modulation::QPSK,
        .frequency = 12187500,
        .symbolrate = 27500000,
        .horizontal = true,
        .innerFEC = fe_code_rate::FEC_3_4,
        .s2 = false,
        .rolloff = fe_rolloff::ROLLOFF_AUTO,
    };

    SatelliteTune zdfhd = {
        .lnbMode = diseqc_modes::diseqc1,
        .lnb1 = 9750000,
        .lnb2 = 10600000,
        .lnbSwitch = 11700000,
        .lnbPower = true,
        .modulation = fe_modulation::PSK_8,
        .frequency = 11361750,
        .symbolrate = 22000000,
        .horizontal = true,
        .innerFEC = fe_code_rate::FEC_2_3,
        .s2 = true,
        .rolloff = fe_rolloff::ROLLOFF_35,
    };

    SatelliteTune arte = {
        .lnbMode = diseqc_modes::diseqc1,
        .lnb1 = 9750000,
        .lnb2 = 10600000,
        .lnbSwitch = 11700000,
        .lnbPower = true,
        .modulation = fe_modulation::QPSK,
        .frequency = 10743750,
        .symbolrate = 22000000,
        .horizontal = true,
        .innerFEC = fe_code_rate::FEC_5_6,
        .s2 = false,
        .rolloff = fe_rolloff::ROLLOFF_AUTO,
    };

    SatelliteTune e4p1 = {
        .lnbMode = diseqc_modes::diseqc2,
        .lnb1 = 9750000,
        .lnb2 = 10600000,
        .lnbSwitch = 11700000,
        .lnbPower = true,
        .modulation = fe_modulation::QPSK,
        .frequency = 10936500,
        .symbolrate = 22000000,
        .horizontal = false,
        .innerFEC = fe_code_rate::FEC_5_6,
        .s2 = false,
        .rolloff = fe_rolloff::ROLLOFF_AUTO,
    };

    SatelliteTune radio = {
        .lnbMode = diseqc_modes::diseqc1,
        .lnb1 = 9750000,
        .lnb2 = 10600000,
        .lnbSwitch = 11700000,
        .lnbPower = true,
        .modulation = fe_modulation::QPSK,
        .frequency = 12265500,
        .symbolrate = 27500000,
        .horizontal = true,
        .innerFEC = fe_code_rate::FEC_3_4,
        .s2 = false,
        .rolloff = fe_rolloff::ROLLOFF_AUTO,
    };

    auto tr = frontend_request::tune;

    ::write(fd, &tr, sizeof(tr));
    ::write(fd, &zdfhd, sizeof(SatelliteTune));

    auto addsect = frontend_request::add_stream_filter;
    __u16 pid = 0x2000;

    ::write(fd, &addsect, sizeof(addsect));
    ::write(fd, &pid, sizeof(pid));

    auto wr = ::open("dump.bin", O_CREAT | O_TRUNC | O_WRONLY, 0x777);
    size_t total = 0;

    for (int end = ::time(nullptr) + 60; ::time(nullptr) < end;)
    {
        response response;

        auto resp = readBuffer(fd, &response, sizeof(response));

        if (resp != sizeof(response))
            break;

        auto buffer = ::malloc(response.len);
        auto bytes = readBuffer(fd, buffer, response.len);

        if (bytes == response.len && response.type == frontend_response::stream)
        {
            ::write(wr, buffer, bytes);

            total += bytes;
        }

        ::free(buffer);

        if (bytes != response.len)
            break;
    }

    ::close(wr);

    printf("done %ld\n", total);

    ::close(fd);
#endif

    manager.run();
}
