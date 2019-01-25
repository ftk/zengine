#include "main.hpp"

#include <cstdlib>
#include <ctime>

#include "components/config.hpp"
#include "util/log.hpp"

#ifdef WIN32 // for chdir
#include <direct.h>
#else
#include <unistd.h>
#endif


application * g_app = nullptr;

z_main::z_main(int argc, const char * const * argv)
{
    srand(time(nullptr));
    make_current();

    g_app->config.load<config_c>("config.txt", argc, argv);

    if(auto dir = static_config::at_optional<std::string>(ID("workdir")))
        chdir(dir->c_str());
#if !defined(LOG_DISABLE) && !defined(LOG_HEADER_ONLY)
    if(auto filename = static_config::at_optional<std::string>(ID("log.file")))
    {
        static std::ofstream logfile(*filename);
        loggers().push_back({logfile, log_level::all, log_detail::TIME});
    }
#endif
}

z_main::~z_main()
{
    LOGGER(info, "Unloading application...");
}

void z_main::run()
{
    g_app->init_components();
    try
    {
        g_app->run();
    }
    catch(const std::exception& e)
    {
        LOGGER(fatal, "Uncaught exception:", e.what());
        throw;
    }

}
