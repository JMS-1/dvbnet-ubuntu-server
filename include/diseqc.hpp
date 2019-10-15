#include <linux/types.h>

enum diseqc_modes
{
    burst_off,
    burst_on,
    diseqc1,
    diseqc2,
    diseqc3,
    diseqc4,
    none,
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