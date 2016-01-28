
#include <boost/test/unit_test.hpp>

#include <sstream>

#include "log.hpp"
#include "potatocache.hpp"

using namespace potatocache;
using namespace std;

BOOST_AUTO_TEST_CASE(test_logging_off)
{
   api::set_log_output(nullptr);
   api::set_log_level(log_level::INFO);
   LOG_DEBUG("debug");
   LOG_INFO("info");
   LOG_WARNING("warning");
   LOG_ERROR("error");
}

BOOST_AUTO_TEST_CASE(test_logging_all_levels_allowed)
{
   stringstream ss;
   api::set_log_output(&ss);
   api::set_log_level(log_level::DEBUG);
   
   LOG_DEBUG("debug");
   LOG_INFO("info");
   LOG_WARNING("warning");
   LOG_ERROR("error");

   string str = ss.str();
   BOOST_CHECK(str.find("debug")   != string::npos);
   BOOST_CHECK(str.find("info")    != string::npos);
   BOOST_CHECK(str.find("warning") != string::npos);
   BOOST_CHECK(str.find("error")   != string::npos);
}

BOOST_AUTO_TEST_CASE(test_logging_info_level)
{
   stringstream ss;
   api::set_log_output(&ss);
   api::set_log_level(log_level::INFO);
   
   LOG_DEBUG("debug");
   LOG_INFO("info");
   LOG_WARNING("warning");
   LOG_ERROR("error");

   string str = ss.str();
   BOOST_CHECK(str.find("debug")   == string::npos);
   BOOST_CHECK(str.find("info")    != string::npos);
   BOOST_CHECK(str.find("warning") != string::npos);
   BOOST_CHECK(str.find("error")   != string::npos);
}

BOOST_AUTO_TEST_CASE(test_logging_warning_level)
{
   stringstream ss;
   api::set_log_output(&ss);
   api::set_log_level(log_level::WARNING);
   
   LOG_DEBUG("debug");
   LOG_INFO("info");
   LOG_WARNING("warning");
   LOG_ERROR("error");

   string str = ss.str();
   BOOST_CHECK(str.find("debug")   == string::npos);
   BOOST_CHECK(str.find("info")    == string::npos);
   BOOST_CHECK(str.find("warning") != string::npos);
   BOOST_CHECK(str.find("error")   != string::npos);
}

BOOST_AUTO_TEST_CASE(test_logging_error_level)
{
   stringstream ss;
   api::set_log_output(&ss);
   api::set_log_level(log_level::ERROR);
   
   LOG_DEBUG("debug");
   LOG_INFO("info");
   LOG_WARNING("warning");
   LOG_ERROR("error");

   string str = ss.str();
   BOOST_CHECK(str.find("debug")   == string::npos);
   BOOST_CHECK(str.find("info")    == string::npos);
   BOOST_CHECK(str.find("warning") == string::npos);
   BOOST_CHECK(str.find("error")   != string::npos);
}
