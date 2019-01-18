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
#include <limits>
#include "util/assert.hpp"
#include "util/movable.hpp"

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

    template <typename R, typename... Args>
    class signal;

    class connection
    {
    protected:
        std::function<void ()> disconnector;
    public:
        explicit connection(std::function<void ()> disconnector) : disconnector{std::move(disconnector)} {}
        connection() = default;
    public:

        void disconnect() noexcept
        {
            if(connected())
                disconnector();
            disconnector = nullptr;
        }

        bool connected() const noexcept
        {
            return static_cast<bool>(disconnector);
        }
    };

    class scoped_connection : public connection
    {
        NONCOPYABLE(scoped_connection)
    public:
        scoped_connection(connection c) : connection(std::move(c)) {}
        scoped_connection() = default;
        scoped_connection(scoped_connection&& other) = default;
        scoped_connection& operator=(scoped_connection&& other) noexcept
        {
            if(&other == this) return *this;
            disconnect();
            connection::operator=(std::move(other));
            return *this;
        }
        scoped_connection& operator=(const connection &rhs) noexcept
        {
            disconnect();
            connection::operator=(rhs);
            return *this;
        }
        ~scoped_connection() noexcept
        {
            disconnect();
        }
    };

    enum connect_position { at_front, at_back };


    template <typename R, typename... Args>
    class signal<R(Args...)> : protected basic_signal<R(Args...)>
    {
        using base = basic_signal<R(Args...)>;
        using GroupKey = int;
    protected:
        struct slot_info
        {
            int id;
            GroupKey group;
            bool operator < (const slot_info& rhs) const { return group < rhs.group; }
        };
        std::vector<slot_info> slots_info;
        int id_counter = 0;
        NONCOPYABLE(signal)
    public:
        signal() = default;

        using base::operator();
        using base::size;

        // do not call connect on the same signal from the slots
        template <typename F>
        connection connect(F&& f, connect_position pos = at_back)
        {
            return connect(std::forward<F>(f), (pos == at_back) ? std::numeric_limits<GroupKey>::max() : std::numeric_limits<GroupKey>::min(), pos);
        }

        template <typename F>
        connection connect(F&& f, GroupKey group, connect_position pos = at_back)
        {
            assume(slots_info.size() == this->size());

            int i = id_counter;
            auto info = slot_info{i, group};
            typename std::vector<slot_info>::iterator info_it;
            if(pos == at_back)
                info_it = std::upper_bound(slots_info.begin(), slots_info.end(), info);
            else
                info_it = std::lower_bound(slots_info.begin(), slots_info.end(), info);

            this->insert(this->begin() + (info_it - slots_info.begin()), std::forward<F>(f));
            slots_info.insert(info_it, std::move(info));

            id_counter++;
            return connection{[this,i](){disconnect(i);}};
        }

        void clear()
        {
            base::clear();
            slots_info.clear();
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
