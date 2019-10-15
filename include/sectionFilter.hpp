#include "filter.hpp"

class SectionFilter : public Filter
{
    friend class Frontend;

private:
    SectionFilter(Frontend &frontend, __u16 pid) : Filter(frontend, pid, "dump.epg", 1000, 200000) {}

public:
    bool start();
};
