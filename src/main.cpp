#include "manager.hpp"

int main()
{
    FrontendManager manager;

    if (!manager.listen(29714))
    {
        ::exit(1);
    }

    ::printf("listener started\n");

    for (;;)
    {
        ::sleep(1000);
    }
}
