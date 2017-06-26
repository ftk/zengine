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


#include "opengl/math.hpp"

int main(int argc, char * argv[])
{
    srand(time(nullptr));

    auto alloc = std::allocator<application>();
    g_app = alloc.allocate(1);
    alloc.construct(g_app);

    g_app->config.load();

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
            g_app->config->configfile = value.to_string();
        else
            g_app->config->set(key.to_string(), value.to_string());
    }
    // start in app's dir
    if(argn != argc)
        chdir(argv[argn++]);
    //if(SDL_GetBasePath())
        //chdir(SDL_GetBasePath());

    if(!g_app->config->configfile.empty())
        g_app->config->load_from_file(g_app->config->configfile);

    if(auto filename = g_app->config->get_optional<std::string>("log.file"))
    {
        std::ofstream logfile(*filename);
        loggers().push_back({logfile, log_level::all, log_detail::TIME});
    }


    try
    {
        g_app->init_components();
        try
        {
            g_app->run();
        }
        catch(const std::exception& e)
        {
            std::cerr << "Uncaught exception: " << e.what() << std::endl;
            LOGGER(fatal, "Uncaught exception:", e.what());
            throw;
        }
        LOGGER(info, "Unloading application...");
        alloc.destroy(g_app);
        alloc.deallocate(g_app, 1);

        return 0;
    }
    catch(...)
    {
        alloc.destroy(g_app);
        alloc.deallocate(g_app, 1);
        throw;
    }

}
