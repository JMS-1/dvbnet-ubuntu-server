#include "manager.hpp"

int main()
{
    FrontendManager manager;

    if (!manager.listen())
    {
        ::exit(1);
    }

    ::printf("listener started\n");

    manager.run();
}
