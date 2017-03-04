#include <iostream>
#include <cstdlib>
#include <ctime>


#ifdef WIN32
#include <direct.h>
#include <algorithm>
#include <fstream>

#else
#include <unistd.h>
#endif


#include "application.hpp"

#include "util/log.hpp"


application * g_app = nullptr;


int main(int argc, char * argv[])
try
{
    //std::cout << sizeof(long) << '\n';
    srand(time(nullptr));

    /*
    auto l1 = flist<int>{1,2,3,4,5};
    auto l2 = 1 >>= 2 >>= 3 >>= 4 >>= 5 >>= flist<int>{};
    auto l3 = +l1; // O(n), l3 now has a copy of l1
    auto one = l1.pop(); // first = 1, l1 = {2,3,4,5}
    l1 = -l1 + -l2; // O(1), l1 now contains {2,3,4,5,1,2,3,4,5}, l2 is empty

    auto l4 = (+l1).map([](int v)->float{return v / 2.f;});
    l4.for_each([](auto& b) { std::cout << b << ' ';});
    auto l5 = (-l1).map([](int v)->int{return v * 2;});
    l5.for_each([](auto& b) { std::cout << b << ' ';});
*/




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
    }
    catch(...)
    {
        alloc.destroy(g_app);
        alloc.deallocate(g_app, 1);
        throw;
    }
    //g_app->~application();

    alloc.destroy(g_app);
    alloc.deallocate(g_app, 1);

    return 0;
}
catch(const std::exception& e)
{
    std::cerr << "Uncaught exception: " << e.what() << std::endl;
    throw;
    return 1;
}