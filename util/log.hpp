//
// Created by fotyev on 2017-02-04.
//

#ifndef ZENGINE_LOG_HPP
#define ZENGINE_LOG_HPP

#include <iostream>

static constexpr auto& log_stream = std::cerr;

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
constexpr log_level cur_level = log_level::debug2;

namespace detail {
template <typename T>
inline void log_arg(log_level level, T&& t)
{
    log_stream << std::forward<T>(t) << ' ';
}
// specializations?


inline void log_start(log_level level) {}

inline void log_args(log_level level)
{
}

template <typename Arg1, typename... Args>
inline void log_args(log_level level, Arg1&& arg1, Args&& ... args)
{
    detail::log_arg(level, std::forward<Arg1>(arg1));
    log_args(level, std::forward<Args>(args)...);

}

inline void log_end(log_level level)
{
    // end of line
    log_stream << std::endl;
}

} // namespace detail




template <typename... Args>
inline void logger(log_level level, Args&&... args)
{
    if(level <= cur_level)
    {
        detail::log_start(level);
        detail::log_args(level, std::forward<Args>(args)...);
        detail::log_end(level);
    }
};

#define LOGGER(level,...) logger(log_level:: level, ##__VA_ARGS__)


#endif //ZENGINE_LOG_HPP
