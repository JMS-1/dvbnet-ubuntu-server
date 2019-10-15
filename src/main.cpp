#include "manager.hpp"

#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

void reader(int fd)
{
    auto dump = ::open("dump.bin", O_WRONLY | O_CREAT | O_TRUNC);

    char buffer[100000];

    for (int total = 0;;)
    {
        auto len = ::read(fd, buffer, sizeof(buffer));

        if (len <= 0)
        {
            ::close(dump);

            ::printf("total of %d\n", total);

            break;
        }

        ::write(dump, buffer, len);

        total += len;
    }
}

int main()
{
    FrontendManager manager;

    manager.listen(29714);

    auto server = ::gethostbyname("localhost");

    sockaddr_in addr = {.sin_family = AF_INET, .sin_port = 29714, {0}};

    ::memcpy(&addr.sin_addr.s_addr, server->h_addr, server->h_length);

    auto fd = socket(AF_INET, SOCK_STREAM, 0);
    auto err = ::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));

    if (err != 0)
    {
        printf("connect: %d %d\n", err, errno);
    }

    auto thread = new std::thread(reader, fd);

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

    auto tr = frontend_request::tune;

    ::write(fd, &tr, sizeof(tr));
    ::write(fd, &rtlplus, sizeof(rtlplus));

    ::sleep(1);

    auto addsect = frontend_request::add_section_filter;
    __u16 epg = 18;

    ::write(fd, &addsect, sizeof(addsect));
    ::write(fd, &epg, sizeof(epg));

    ::sleep(5);

    auto delsect = frontend_request::del_filter;

    ::write(fd, &delsect, sizeof(delsect));
    ::write(fd, &epg, sizeof(epg));

    auto addstream = frontend_request::add_stream_filter;
    __u16 vid = 168;

    ::write(fd, &addstream, sizeof(addstream));
    ::write(fd, &vid, sizeof(vid));

    ::sleep(10);

    printf("done\n");

    ::close(fd);

    if (thread->joinable())
    {
        thread->join();
    }
}
