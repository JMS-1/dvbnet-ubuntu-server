#include "filter.hpp"

class StreamFilter : public Filter
{
    friend class Frontend;

private:
    StreamFilter(Frontend &frontend, __u16 pid) : Filter(frontend, pid, 10000, "dump.vid") {}

public:
    bool start();
};