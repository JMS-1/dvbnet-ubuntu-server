#ifndef _DVBNET_DISEQC_H
#define _DVBNET_DISEQC_H 1

#include <sys/types.h>

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
protected:
    // Erzeugt eine Burst-Mode Steuermeldung.
    DiSEqCMessage() : burst(false), repeat(0) {}

public:
    virtual ~DiSEqCMessage() {}

    // Erzeugt eine Burst-Mode Steuermeldung.
    DiSEqCMessage(bool burst) : burst(burst), repeat(0) {}

    // Erzeugt eine Standardsteuermeldung.
    DiSEqCMessage(u_char repeat, u_char cmd0, u_char cmd1, u_char cmd2, u_char cmd3 = 0) : burst(false), repeat(repeat)
    {
        _message[0] = cmd0;
        _message[1] = cmd1;
        _message[2] = cmd2;
        _message[3] = cmd3;
    }

private:
    // Nachricht - falls nicht der Burst-Mode verwendet wird.
    u_char _message[4];
    // Burst-Mode - nur berücksichtigt wenn die Anzahl der Wiederholungen 0 ist.
    const bool burst;
    // Die Anzahl der Wiederholungen - zurzeit nicht verwendet.
    const u_char repeat;

public:
    // Überträgt die Meldung an ein Frontend.
    virtual void _send(int fd);

public:
    // Erstellt eine neue Steuermeldung.
    static void send(diseqc_modes mode, bool highFrequency, bool horizontal, int fe);
};

#endif