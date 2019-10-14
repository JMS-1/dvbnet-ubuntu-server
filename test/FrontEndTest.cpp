#define BOOST_TEST_MODULE FrontendTest

#include "frontend.hpp"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(FrontendSuite)

BOOST_AUTO_TEST_CASE(AdditionTest)
{
    Frontend frontend(0, 0);

    BOOST_CHECK_EQUAL(frontend.open(), true);
    BOOST_CHECK_EQUAL(frontend.getStatus(), fe_status::FE_NONE);
    BOOST_CHECK_EQUAL(frontend.tune(), 0);

    ::sleep(1);

    BOOST_CHECK_EQUAL(frontend.getStatus(), 31);

    auto &filter = frontend.createSectionFilter(18);

    filter.start();
}

BOOST_AUTO_TEST_SUITE_END()