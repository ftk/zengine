#include <iostream>
#include <cstdlib>
#include <ctime>

#include <algorithm>
#include <fstream>

#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "application.hpp"

#include "util/log.hpp"


application * g_app = nullptr;


int main(int argc, char * argv[])
try
{
    srand(time(nullptr));


    // start in app's dir
    if(argc > 1)
        chdir(argv[1]);
    //if(SDL_GetBasePath())
        //chdir(SDL_GetBasePath());

    std::ofstream logfile("log.txt");
    loggers().push_back({logfile, log_level::all, log_detail::TIME});

    auto alloc = std::allocator<application>();
    g_app = alloc.allocate(1);
    alloc.construct(g_app);

    try
    {
        g_app->init_components();
        g_app->run();

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
catch(const std::exception& e)
{
    std::cerr << "Uncaught exception: " << e.what() << std::endl;
    throw;
    return 1;
}