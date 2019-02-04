// compile boost.system
#include <boost/version.hpp>

#if BOOST_VERSION < 106900
#define BOOST_SYSTEM_SOURCE
#include <boost/system/error_code.hpp>
#include <boost/system/detail/error_code.ipp>
#endif

#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include <boost/asio/impl/src.hpp>
#endif

