#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>

class Frontend;

class Filter
{
protected:
    Filter(Frontend &frontend, __u16 pid) : _frontend(frontend), _connected(true), _pid(pid), _fd(-1) {}

public:
    virtual ~Filter() { _connected = false; }

private:
    bool _connected;
    Frontend &_frontend;

protected:
    const __u16 _pid;
    int _fd;

public:
    virtual void start() = 0;
    virtual void stop() = 0;
};

class Frontend
{
public:
    Frontend(int adapter, int frontend) : _adapter(adapter), _frontend(frontend), _fd(-1)
    {
    }

    ~Frontend()
    {
        close();
    }

private:
    const int _adapter;
    const int _frontend;
    int _fd;

public:
    const bool isOpen() { return _fd >= 0; }

public:
    bool close();
    bool open();
    const fe_status getStatus();
    Filter &createSectionFilter(__u16 pid);
    int tune();
};

class SectionFilter : public Filter
{
    friend class Frontend;

private:
    SectionFilter(Frontend &frontend, __u16 pid) : Filter(frontend, pid) {}

public:
    ~SectionFilter()
    {
        stop();
    }

public:
    void start();
    void stop();
};