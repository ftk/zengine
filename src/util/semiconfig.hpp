//
// Created by fotyev on 2018-12-02.
//

#ifndef ZENGINE_SEMICONFIG_HPP
#define ZENGINE_SEMICONFIG_HPP


#include <unordered_map>
#include <string>
#include <optional>

#include <boost/lexical_cast.hpp>

#include "string.hpp"

#define ID(x) []() constexpr { return x; }

#ifndef CONFIG_ON_CAST_ERROR
#ifdef CONFIG_ON_CAST_ERROR_THROW
#define CONFIG_ON_CAST_ERROR(msg) throw std::runtime_error{msg};
#else
#include "log.hpp"
#define CONFIG_ON_CAST_ERROR(msg) LOGGER(error, msg)
#define CONFIG_NOEXCEPT noexcept
#endif
#endif

#ifndef CONFIG_NOEXCEPT
#define CONFIG_NOEXCEPT
#endif


// both static and dynamic configuration
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
    static T& at(Id id, T def) CONFIG_NOEXCEPT
    {
        if(at_optional<T>(id))
            return *static_storage<T, Id>;
        return static_storage<T, Id>.emplace(std::move(def));
    }

    template <typename T, typename Id>
    static std::optional<T>& at_optional(Id id) CONFIG_NOEXCEPT
    {
        if(static_storage<T, Id>)
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
                future_write.erase(id());
            }
            catch(const boost::bad_lexical_cast& e)
            {
                cast_error(id(), e.what(), future_write[id()].c_str());
            }
        }
        return static_storage<T, Id>;
    }

    // only write static variables
    static void write_ini_static(std::ostream& s) CONFIG_NOEXCEPT
    {
        for(const auto& p : static_access)
        {
            try
            {
                s << p.first << '=' << p.second.read() << '\n';
            }
            catch(const boost::bad_lexical_cast& e)
            {
                cast_error(p.first, e.what());
            }
        }
    }
    static void write_ini_dynamic(std::ostream& s) CONFIG_NOEXCEPT
    {
        for(const auto& p : future_write)
        {
            s << p.first << '=' << p.second << '\n';
        }
    }

    static void read_ini(std::istream& s) CONFIG_NOEXCEPT
    {
        std::string line;
        while(std::getline(s, line))
        {
            if(line.empty() || line[0] == '#' || line[0] == ';')
                continue;
            auto spl = string_split<2>(line, '=');
            if(!spl[1].empty())
                set(string{spl[0]}, string{spl[1]});
        }
    }

    // read only dynamic access
    static string get(const string& id) CONFIG_NOEXCEPT
    {
        if(auto opt = get_optional(id))
            return *opt;
        return "";
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

    // return a map of variables and their values for whom at(...) was not called yet
    static const std::unordered_map<string, string>& dynamic_vars() noexcept { return future_write; }

private:
    static void cast_error(const string& id, const char * what, const char * write = nullptr) CONFIG_NOEXCEPT
    {
        if(write)
            CONFIG_ON_CAST_ERROR("config: error writing variable " + id + " to " + write + ": " + what);
        else
            CONFIG_ON_CAST_ERROR("config: error reading variable" + id +  ": " + what);
    }
};

#endif //ZENGINE_SEMICONFIG_HPP
