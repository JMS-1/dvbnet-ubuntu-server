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
    DiSEqCMessage(bool burst) : message(nullptr), burst(burst), repeat(1) {}
    // Erzeugt eine Standardsteuermeldung.
    DiSEqCMessage(__u8 message[], __u8 repeat) : message(message), burst(false), repeat(repeat) {}

public:
    // Nachricht - falls nicht der Burst-Mode verwendet wird.
    const __u8 *message;
    // Burst-Mode - nur berücksichtigt wenn die Nachricht nicht gesetzt ist.
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