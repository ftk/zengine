#ifdef NO_BOOST_SYSTEM
#define BOOST_ERROR_CODE_HEADER_ONLY
#endif

#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include <boost/asio/impl/src.hpp>
#endif

#ifdef NO_BOOST_SYSTEM
#include <boost/system/error_code.hpp> // compile boost.system in this file
#undef BOOST_ERROR_CODE_HEADER_ONLY
#endif