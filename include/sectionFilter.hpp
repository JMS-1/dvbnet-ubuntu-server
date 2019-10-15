#include "filter.hpp"

class SectionFilter : public Filter
{
    friend class Frontend;

private:
    SectionFilter(Frontend &frontend, __u16 pid) : Filter(frontend, pid, frontend_response::section) {}

public:
    bool start();
};
