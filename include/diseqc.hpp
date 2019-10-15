#ifndef _DVBNET_DISEQC_H
#define _DVBNET_DISEQC_H 1

#include <linux/types.h>

enum diseqc_modes
{
    burst_off = 5,
    burst_on = 6,
    diseqc1 = 1,
    diseqc2 = 2,
    diseqc3 = 3,
    diseqc4 = 4,
    none = 0,
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
    int send(int fd);

public:
    static DiSEqCMessage create(diseqc_modes mode, bool highFrequency, bool horizontal);
};

#endif