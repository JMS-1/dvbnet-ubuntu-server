#include "filter.hpp"

class StreamFilter : public Filter
{
    friend class Frontend;

private:
    StreamFilter(Frontend &frontend, __u16 pid) : Filter(frontend, pid, "dump.vid", 10000, 2000000) {}

public:
    bool start();
};