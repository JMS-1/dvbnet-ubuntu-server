#ifndef _DVBNET_STREAM_FILTER_H
#define _DVBNET_STREAM_FILTER_H 1

#include "filter.hpp"

class StreamFilter : public Filter
{
    friend class Frontend;

private:
    StreamFilter(Frontend &frontend, __u16 pid) : Filter(frontend, pid, frontend_response::stream) {}

public:
    bool start();
};

#endif