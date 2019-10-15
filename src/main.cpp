#include "frontend.hpp"

#include <unistd.h>

int main()
{
    Frontend frontend(0, 0);

    printf("open: %d\n", frontend.open());
    printf("getStatus: %d\n", frontend.getStatus());
    printf("tune: %d\n", frontend.tune());

    ::sleep(1);

    printf("getStatus: %d\n", frontend.getStatus());

    frontend.createSectionFilter(18);
    printf("startFilter: %d\n", frontend.startFilter(18));

    ::sleep(5);

    printf("removeFilter: %d\n", frontend.removeFilter(18));

    frontend.createStreamFilter(168);
    printf("startFilter: %d\n", frontend.startFilter(168));

    ::sleep(10);

    printf("done\n");
}
