#include "filter.hpp"

class StreamFilter : public Filter
{
    friend class Frontend;

private:
    StreamFilter(Frontend &frontend, __u16 pid) : Filter(frontend, pid, frontend_response::stream) {}

public:
    bool start();
};