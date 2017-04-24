//
// Created by fotyev on 2017-04-23.
//

// compile whole project with -finstrument-functions -g except this file for stacktrace to work
// requires addr2line tool at runtime

#include <string>

void print_backtrace(); // print stack of the current thread
std::string get_exe_path();


#if !defined(BACKTRACE) && defined(NDEBUG)
#define NO_BACKTRACE
#endif

#ifdef NO_BACKTRACE
void print_backtrace() {}
#else

#if 1
#include <boost/container/static_vector.hpp>
using stack_type = boost::container::static_vector<void *, 512>;
#else
#include <vector>
using stack_type = std::vector<void *>;
#endif

#include <cstdlib>
#include <iostream>
#include <sstream>

static thread_local stack_type bt;


extern "C" void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
    try
    {
        bt.push_back(call_site);
        bt.push_back(this_fn);
    }
    catch(const std::bad_alloc&)
    {
        // a secret message about bad_alloc
        if(!bt.empty()) bt[0] = nullptr;
    }
}

extern "C" void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
    bt.pop_back();
    bt.pop_back();
}


void print_backtrace()
{
    if(bt.empty())
        return; // backtrace unavailable


    std::ostringstream cmd;
    cmd << "addr2line -f -C -p -e " << get_exe_path();
    for(auto addr : bt)
    {
        cmd << ' ' << addr;
    }
    std::cerr << cmd.str() << std::endl;
    system(cmd.str().c_str());

}

// optional - catch sigsegv
#include <csignal>

static void bt_sighandler(int sig/*, siginfo_t * info, void * ctx*/)
{
    std::cerr << "Signal " << sig << std::endl;
    print_backtrace();
    std::exit(sig);
}


static void signal_handler_register() __attribute__((constructor));
static void signal_handler_register()
{
/*
    struct sigaction sa;

    sa.sa_sigaction = &bt_sighandler;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    */
    signal(SIGSEGV, &bt_sighandler);
#ifndef WIN32
    //signal(SIGUSR1, &bt_sighandler);
#endif
}

// optional - catch assertion fails

#ifndef NO_SDL
#include "assert.hpp"

static SDL_AssertState assert_handler(const SDL_AssertData* data, void* userdata)
{
    print_backtrace();
    return SDL_GetDefaultAssertionHandler()(data, userdata);
}

static void assert_handler_register() __attribute__((constructor));
static void assert_handler_register()
{
    SDL_SetAssertionHandler(assert_handler, nullptr);
}

#endif // NO_SDL


#endif // NO_BACKTRACE

#ifdef WIN32
#include <Windows.h>

std::string get_exe_path()
{
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    return buffer;
}
#else
#include <sys/types.h>
#include <unistd.h>

std::string get_exe_path()
{
    char link[64];
    char buffer[1024];
    snprintf(link, sizeof link, "/proc/%d/exe", getpid());
    int len = readlink(link, buffer, sizeof(buffer)-1);
    if(len == -1)
        return std::string{};
    buffer[len] = '\0';
    return buffer;
}
#endif

