#include "frontend.hpp"

#include <unistd.h>

void reportSignal(const Frontend &frontend)
{
    auto &signal = frontend.getSignal();

    printf("status=%d strength=%d snr=%d ber=%d\n", signal.status, signal.strength, signal.snr, signal.ber);
}

int main()
{
    Frontend frontend(0, 0);

    printf("open: %d\n", frontend.open());

    reportSignal(frontend);

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

    printf("tune: %d\n", frontend.tune(rtlplus));

    ::sleep(1);

    reportSignal(frontend);

    frontend.createSectionFilter(18);
    printf("startFilter: %d\n", frontend.startFilter(18));

    ::sleep(5);

    reportSignal(frontend);

    printf("removeFilter: %d\n", frontend.removeFilter(18));

    frontend.createStreamFilter(168);
    printf("startFilter: %d\n", frontend.startFilter(168));

    ::sleep(10);

    reportSignal(frontend);

    printf("done\n");
}
