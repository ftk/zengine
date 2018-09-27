//
// Created by fotyev on 2018-07-12.
//

#ifndef ZENGINE_SIGNALS_HPP
#define ZENGINE_SIGNALS_HPP

#if 0
#include <boost/signals2/signal.hpp>

namespace sig = boost::signals2;

#else

#include <functional>
#include <vector>

namespace sig {
    template <typename R, typename... Args>
    class signal;

    template <typename R, typename... Args>
    class signal<R(Args...)> : std::vector<std::function<R(Args...)>>
    {
    public:
        void connect(auto&& f)
        {
            this->push_back(std::move(f));
        }

       // template <typename... Args2>
        void operator ()(Args... args)
        {
            for(auto& f : (*this))
                f(args...);
        }
    };


}

#endif

#endif //ZENGINE_SIGNALS_HPP
