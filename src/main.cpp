#include <iostream>
#include <cstdlib>
#include <ctime>

#include <algorithm>
#include <fstream>

#ifdef WIN32 // for chdir
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "application.hpp"

#include "components/config.hpp"

#include "util/log.hpp"
#include "util/hash.hpp"

application * g_app = nullptr;

int main(int argc, char * argv[])
{
    srand(time(nullptr));

    auto app = std::make_unique<application>();
    g_app = app.get();

    g_app->config.load();
    g_app->config->load_from_file("config.xml");

    int argn;
    for(argn = 1; argn < argc; argn++)
    {
        auto arg = string_view{argv[argn]};
        std::size_t eq_pos = arg.find('=');
        if(eq_pos == string_view::npos)
            break;

        auto key = arg.substr(0, eq_pos);
        auto value = arg.substr(eq_pos + 1);

        if(key == "file")
        {
            if(!value.empty())
            {
                g_app->config->load_from_file(value);
            }
        } else
            g_app->config->set(std::string{key}, std::string{value});
    }
    // start in app's dir
    if(argn != argc)
        chdir(argv[argn++]);

    if(auto filename = g_app->config->get_optional<std::string>("log.file"))
    {
        static std::ofstream logfile(*filename);
        loggers().push_back({logfile, log_level::all, log_detail::TIME});
    }

    g_app->init_components();
    try
    {
        void init_game();
        init_game();
        g_app->run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Uncaught exception: " << e.what() << std::endl;
        LOGGER(fatal, "Uncaught exception:", e.what());
        throw;
    }
    LOGGER(info, "Unloading application...");
    return 0;
}
