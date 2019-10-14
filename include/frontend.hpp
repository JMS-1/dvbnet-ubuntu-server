#include <linux/dvb/frontend.h>

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
    const fe_status getStatus();
    bool close();
    bool open();
    int tune();
};