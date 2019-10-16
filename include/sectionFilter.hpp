#ifndef _DVBNET_SECTION_FILTER_H
#define _DVBNET_SECTION_FILTER_H 1

#include "filter.hpp"

/*
    Klasse zur Entgegennahme eines Kontrolldatenstroms.
*/
class SectionFilter : public Filter
{
    friend class Frontend;

private:
    SectionFilter(Frontend &frontend, __u16 pid) : Filter(frontend, pid, frontend_response::section) {}

public:
    // Aktiviert den Empfang.
    bool start();
};

#endif
