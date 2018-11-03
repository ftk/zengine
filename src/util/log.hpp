//
// Created by fotyev on 2017-02-04.
//

#ifndef ZENGINE_LOG_HPP
#define ZENGINE_LOG_HPP

#ifndef LOG_DISABLE

#include <ostream>

#ifdef LOG_HEADER_ONLY
#include <iostream>
#else
#include <vector>
#endif

enum class log_level
{
    fatal = 10,
    error = 20,
    warn = 30,
    info = 40,
    debug = 61,
    debug2 = 62,
    debug3 = 63,
    debug4 = 64,
    debug5 = 65,
    all = 100,
};

namespace log_detail {

enum // flags
{
    NO_NEWLINE = 1u << 0,
    NO_FLUSH = 1u << 1,
    TIME = 1u << 2,

};

struct logger
{
    std::ostream& s;
    log_level max;
    unsigned flags;
};

#ifdef LOG_HEADER_ONLY
#ifndef LOG_STATIC_LOGGERS
#define LOG_STATIC_LOGGERS { std::cerr, log_level::debug2, 0 }
#endif
static constexpr logger loggers[] { LOG_STATIC_LOGGERS };
#else
extern std::vector<logger> loggers;
#endif

template <typename T>
inline void log_arg(const logger& l, T&& t) noexcept
{
    l.s << std::forward<T>(t) << ' ';
}
// specializations?


#ifdef LOG_HEADER_ONLY
inline constexpr
#endif
void log_start(const logger& l) noexcept
#ifdef LOG_HEADER_ONLY
{}
#endif
;

#ifdef LOG_HEADER_ONLY
inline
#endif
void log_end(const logger& l) noexcept
#ifdef LOG_HEADER_ONLY
{
    l.s << std::endl;
}
#endif
;


inline constexpr void log_args(const logger& l) noexcept { }

template <typename Arg1, typename... Args>
inline void log_args(const logger& l, Arg1&& arg1, Args&& ... args) noexcept
{
    log_arg(l, std::forward<Arg1>(arg1));
    log_args(l, std::forward<Args>(args)...);

}

} // namespace log_detail




template <typename... Args>
inline void logger(log_level level, Args&&... args) noexcept
{
    using namespace log_detail;
    for(const auto& l : loggers)
    {
        if(level <= l.max)
        {
            log_start(l);
            log_args(l, std::forward<Args>(args)...);
            log_end(l);
        }
    }
};

inline constexpr auto& loggers() noexcept { return log_detail::loggers; }


#ifdef LOG_FILENAME
#ifdef __GNUC__
#define LOG_FILE (__builtin_strrchr(__FILE__, '/') ? (__builtin_strrchr(__FILE__, '/')+1) : (__builtin_strrchr(__FILE__, '\\') ? (__builtin_strrchr(__FILE__, '\\')+1) : __FILE__))
#else
#include <cstring>
#define LOG_FILE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') : __FILE__)
#endif
#define LOGGER(level,...) logger(log_level:: level, LOG_FILE, ##__VA_ARGS__)
#else
#define LOGGER(level,...) logger(log_level:: level, ##__VA_ARGS__)
#endif

#else // LOG_DISABLE

#define LOGGER(level,...) static_cast<void>(0)

#endif

#endif //ZENGINE_LOG_HPP
