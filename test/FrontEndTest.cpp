#define BOOST_TEST_MODULE FrontendTest

#include "frontend.hpp"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(FrontendSuite)

BOOST_AUTO_TEST_CASE(AdditionTest)
{
    Frontend Frontend(0, 0);

    BOOST_CHECK_EQUAL(Frontend.open(), true);
    BOOST_CHECK_EQUAL(Frontend.getStatus(), fe_status::FE_NONE);
    BOOST_CHECK_EQUAL(Frontend.tune(), 0);

    ::sleep(1);

    BOOST_CHECK_EQUAL(Frontend.getStatus(), 31);
}

BOOST_AUTO_TEST_SUITE_END()