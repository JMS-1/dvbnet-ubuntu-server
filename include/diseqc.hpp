#ifndef _DVBNET_DISEQC_H
#define _DVBNET_DISEQC_H 1

#include <linux/types.h>

// Unterstützte DiSQeC Befehle.
enum diseqc_modes
{
    // Keine Schaltung.
    none = 0,
    // Burst-Mode.
    burst_off = 5,
    burst_on = 6,
    // Standard.
    diseqc1 = 1,
    diseqc2 = 2,
    diseqc3 = 3,
    diseqc4 = 4,
};

/*
    Beschreibt eine DiSEqC Steuermeldung.
*/
class DiSEqCMessage
{
public:
    // Erzeugt eine Burst-Mode Steuermeldung.
    DiSEqCMessage(bool burst) : burst(burst), repeat(0) {}

    // Erzeugt eine Standardsteuermeldung.
    DiSEqCMessage(__u8 repeat, __u8 cmd0, __u8 cmd1, __u8 cmd2, __u8 cmd3 = 0) : burst(false), repeat(repeat)
    {
        _message[0] = cmd0;
        _message[1] = cmd1;
        _message[2] = cmd2;
        _message[3] = cmd3;
    }

private:
    // Nachricht - falls nicht der Burst-Mode verwendet wird.
    __u8 _message[4];
    // Burst-Mode - nur berücksichtigt wenn die Anzahl der Wiederholungen 0 ist.
    const bool burst;
    // Die Anzahl der Wiederholungen - zurzeit nicht verwendet.
    const __u8 repeat;

public:
    // Überträgt die Meldung an ein Frontend.
    int send(int fd);

public:
    // Erstellt eine neue Steuermeldung.
    static DiSEqCMessage create(diseqc_modes mode, bool highFrequency, bool horizontal);
};

#endif