#include "main.hpp"

#include <cstdlib>
#include <ctime>

#include "components/config.hpp"
#include "util/log.hpp"

application * g_app = nullptr;

z_main::z_main(int argc, const char * const * argv)
{
    srand(time(nullptr));
    make_current();

    g_app->config.load<config_c>("config.xml", argc, argv);
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
