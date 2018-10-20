// compile boost.system
#define BOOST_SYSTEM_SOURCE
#include <boost/system/error_code.hpp>
#include <boost/system/detail/error_code.ipp>

#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include <boost/asio/impl/src.hpp>
#endif

