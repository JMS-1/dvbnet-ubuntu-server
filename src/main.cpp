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

    frontend.createSectionFilter(18).start();
    frontend.createStreamFilter(168).start();
}

#define BOOST_TEST_MODULE FrontendTest
