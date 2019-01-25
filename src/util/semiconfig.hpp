
// both static and dynamic configuration system
// requires boost lexical_cast

//  static usage:
//  gets static.var1 or 1 if not exists:
// int& var1 = static_config::at(ID("var1"), 1);
//  if static.var2 exists calls f(var2) and sets its return value to var2
// std::string f(std::string);
// if(auto val =  static_config::at_optional<std::string>(ID("var2")) *val = f(*val);

//  static/dynamic usage:
// static_config::get(std::string{"var1"}) // returns "1"
// bool ok = static_config::set("var3", "6"); // ok
// auto var3 = static_config::at_optional<int>(ID("var3")); // returns optional<int>(6)
// bool ok = static_config::set("var4", "text"); // ok
// int var4 = static_config::at(ID("var4"), 1); // cast_error
// auto var4 = static_config::at_optional<int>(ID("var4")); // returns std::nullopt
// static_config::get("var4"); // returns "text"
// bool ok = static_config::set("var5", "???"); // ok
// static_config::dynamic_vars() // returns {{"var5","???"}} since at(ID("var5")) was not called

/* saving/loading example:
    {
        std::ofstream cfg("config.ini");
        static_config::for_each([&cfg](std::string name, std::string val) { cfg << name << '=' << val << std::endl;; });
    }
    {
        std::ifstream cfg("config.ini");
        std::string line;
        while(std::getline(cfg, line))
        {
            if(line.empty() || line[0] == '#' || line[0] == ';')
                continue;
            while(line.back() == '\r') line.pop_back();
            auto spl = string_split<2>(line, '=');
            if(!spl[1].empty())
                static_config::set(std::string{spl[0]}, std::string{spl[1]});
        }
    }
*/

#ifndef ZENGINE_SEMICONFIG_HPP
#define ZENGINE_SEMICONFIG_HPP


#include <unordered_map>
#include <string>
#include <optional>
#include <type_traits>

#include <boost/lexical_cast.hpp>

#include "const_strid.hpp"
#include "assert.hpp"


#ifndef CONFIG_ON_CAST_ERROR
#if defined(CONFIG_ON_CAST_ERROR_THROW)
#define CONFIG_ON_CAST_ERROR(msg) throw std::runtime_error{msg};
#elif defined(CONFIG_ON_CAST_ERROR_IGNORE)
#define CONFIG_ON_CAST_ERROR(msg) /* ignore */
#define CONFIG_NOEXCEPT noexcept
#else
#include "log.hpp"
#define CONFIG_ON_CAST_ERROR(msg) LOGGER(error, msg)
#define CONFIG_NOEXCEPT noexcept
#endif
#endif

#ifndef CONFIG_NOEXCEPT
#define CONFIG_NOEXCEPT
#endif


class static_config
{
public:
    using string = std::string;
protected:

    static inline std::unordered_map<string, string> future_write; // dynamic id -> value storage. the value will be ejected on static access
    struct access
    {
        string (*read)() = nullptr;
        void (*write)(const string&) = nullptr;
    };
    static inline std::unordered_map<string, access> static_access; // allows access to static storage by runtime ids
    template <typename T, typename Id>
    static inline std::optional<T> static_storage;


public:

    // static access

    template <typename Id, typename T>
    static
    std::enable_if_t<!std::is_same_v<T, const char *>, T&>
    at(Id id, T def) CONFIG_NOEXCEPT
    {
        auto& storage = at_optional<T>(id);
        if(LIKELY(storage))
            return *storage;
        return storage.emplace(std::move(def));
    }

    template <typename Id, typename T>
    static
    std::enable_if_t<std::is_same_v<T, const char *>, string&>
    at(Id id, T def) CONFIG_NOEXCEPT
    {
        auto& storage = at_optional<string>(id);
        if(LIKELY(storage))
            return *storage;
        return storage.emplace(std::move(def));
    }

    template <typename T, typename IdStr>
    static std::optional<T>& at_optional(IdStr id) CONFIG_NOEXCEPT
    {
        using Id = decltype(detail::idval2type(id));

        if(LIKELY(static_storage<T, Id>))
            return static_storage<T, Id>;
        static_access[id()] = {
                []() -> string { return static_storage<T, Id> ? boost::lexical_cast<string>(*static_storage<T, Id>) : ""; },
                [](const string& s) { static_storage<T, Id> = boost::lexical_cast<T>(s); }
        };
        if(future_write.count(id()))
        {
            try
            {
                static_storage<T, Id>.emplace(boost::lexical_cast<T>(future_write[id()]));
            }
            catch(const boost::bad_lexical_cast& e)
            {
                cast_error(id(), e.what(), future_write[id()].c_str());
            }
            future_write.erase(id());
        }
        return static_storage<T, Id>;
    }

    // read only dynamic access
    static string get(const string& id) CONFIG_NOEXCEPT
    {
        return get_optional(id).value_or("");
    }

    static std::optional<string> get_optional(const string& id) CONFIG_NOEXCEPT
    {
        if(static_access.count(id))
        {
            try
            {
                return static_access.find(id)->second.read();
            }
            catch(const boost::bad_lexical_cast& e)
            {
                cast_error(id, e.what());
                return std::nullopt;
            }
        }
        if(future_write.count(id))
        {
            return future_write.find(id)->second;
        }
        return std::nullopt;
    }

    // returns ok if no casting errors occured
    static bool set(const string& id, string value) CONFIG_NOEXCEPT
    {
        if(static_access.count(id))
        {
            try
            {
                static_access[id].write(value);
                return true;
            }
            catch(const boost::bad_lexical_cast& e)
            {
                cast_error(id, e.what(), value.c_str());
                return false;
            }
        }
        future_write[id] = std::move(value);
        return true;
    }

    template <typename F>
    static void for_each(F f) CONFIG_NOEXCEPT // f(string name,string value)
    {
        for(const auto& stat : static_access)
        {
            try
            {
                f(stat.first, stat.second.read());
            }
            catch(const boost::bad_lexical_cast& e)
            {
                cast_error(stat.first, e.what());
            }
        }
        for(const auto& dyn : future_write)
            f(dyn.first, dyn.second);
    }

    static std::size_t size() noexcept { return static_access.size() + future_write.size(); }


    // return a map of variables and their values for whom at(...) was not called yet
    static const auto& dynamic_vars() noexcept { return future_write; }

private:
    static void cast_error(const string& id, const char * what, const char * write = nullptr) CONFIG_NOEXCEPT
    {
        if(write)
            CONFIG_ON_CAST_ERROR("config: error writing variable " + id + " to " + write + ": " + what);
        else
            CONFIG_ON_CAST_ERROR("config: error reading variable" + id +  ": " + what);
    }
};

#undef CONFIG_ON_CAST_ERROR
#undef CONFIG_NOEXCEPT


#define SCFG(cfgname,defvalue) static_config::at(ID(#cfgname), defvalue)

#endif //ZENGINE_SEMICONFIG_HPP
