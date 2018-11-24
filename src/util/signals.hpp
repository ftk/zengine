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

#include "util/assert.hpp"

namespace sig {
    template <typename R, typename... Args>
    class basic_signal;

    template <typename R, typename... Args>
    class basic_signal<R(Args...)> : public std::vector<std::function<R(Args...)>>
    {
    public:
        void operator ()(Args... args) const
        {
            for(auto& f : *this)
                f(args...);
        }

        template <typename F>
        void connect(F&& f)
        {
            this->push_back(std::forward<F>(f));
        }
    };

    struct connection
    {
        std::function<void ()> disconnector;

        void disconnect() noexcept
        {
            disconnector();
            disconnector = nullptr;
        }
    };

    class scoped_connection : public connection
    {
    public:
        scoped_connection(connection c) : connection::connection(std::move(c)) {}
        ~scoped_connection()
        {
            if(disconnector)
                this->disconnect();
        }
    };

    enum connect_position { at_front, at_back };

    template <typename R, typename... Args>
    class signal;

    template <typename R, typename... Args>
    class signal<R(Args...)> : protected basic_signal<R(Args...)>
    {
        using base = basic_signal<R(Args...)>;
    protected:
        struct slot_info
        {
            int id;
        };
        std::vector<slot_info> slots_info;
        int id_counter = 0;
    public:
        using base::operator();
        using base::size;

        template <typename F>
        connection connect(F&& f, connect_position pos = at_back)
        {
            int i = id_counter;
            if(pos == at_back)
            {
                this->push_back(std::forward<F>(f));
                slots_info.push_back({i});
            }
            else
            {
                this->insert(this->begin(), std::forward<F>(f));
                slots_info.insert(slots_info.begin(), {i});
            }
            id_counter++;
            return connection{[this,i](){disconnect(i);}};
        }

    private:
        void disconnect(int id) noexcept
        {
            assume(slots_info.size() == this->size());
            unsigned int i;
            for(i = 0; i < (unsigned int)slots_info.size(); i++)
            {
                if(slots_info[i].id == id)
                    break;
            }
            if(i == (unsigned int)slots_info.size())
                return;
            slots_info.erase(slots_info.begin() + i);
            this->erase(this->begin() + i);
        }

    };

}

#endif

#endif //ZENGINE_SIGNALS_HPP
