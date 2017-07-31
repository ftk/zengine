//
// Created by fotyev on 2017-01-25.
//

#include "script.hpp"
#include <iostream>



#if 1

#include <chaiscript/chaiscript_basic.hpp>

#include "util/hash.hpp"
#include "util/assert.hpp"

#include "util/log.hpp"

using namespace chaiscript;

template <typename R, typename... Args>
R script_callback<R(Args...)>::operator ()(Args... args)
{
    if(f)
    {
        //LOGGER(debug, "called cb: ", args...);
        try
        {
            f(std::forward<Args>(args)...);
        }
        catch(exception::eval_error& e)
        {
            std::cerr << "callback error\n";
            std::cerr << e.pretty_print();
            //assert(false);
        }
    }
}

template <typename R, typename... Args>
void script_callback<R(Args...)>::init(const char * name, ChaiScript_Basic& engine) noexcept
{
    try
    {
        f = engine.eval<decltype(f)>(name);
    }
    catch(exception::eval_error& e)
    {
        LOGGER(warn, "no callback found:", name, e.what());
    }
    catch(exception::bad_boxed_cast& e)
    {
        LOGGER(error, "callback has mismatching signature:", name, e.what());
    }
}

/*< sub uniq { my %seen; grep { !$seen{$_}++ } @_; }
   join "\n", uniq map { qq%template struct script_callback<$_->{type}>;% } dispatch('callbacks');;
   %*/template struct script_callback<void ()>;
template struct script_callback<void (int)>;/*>*/


ModulePtr script_register_bindings(ModulePtr m = std::make_shared<Module>());
ModulePtr script_register_types(ModulePtr m = std::make_shared<Module>());
ModulePtr create_chaiscript_stdlib();
std::unique_ptr<chaiscript::parser::ChaiScript_Parser_Base> create_chaiscript_parser();





class script_c::impl
{
public:
    ChaiScript_Basic chai{create_chaiscript_stdlib(), create_chaiscript_parser()};
    impl()
    {
        register_functions();
    }

    void register_functions()
    {
        chai.add(fun([](const std::string& str) { return fnv1a::hash(str); }), "hash");

        chai.add(
                script_register_bindings(
                        script_register_types()
                )
        );
    }

    void register_callbacks(script_c& p)
    {
#define CB_INIT(name) p. name .init(#name, chai);
        /*< join "\n\t\t", map { qq%CB_INIT($_->{name})% } dispatch('callbacks');
        %*/CB_INIT(on_init)
		CB_INIT(on_option_selected)/*>*/
#undef CB_INIT
    }

    void eval(const std::string& input)
    {
        try
        {
            chai.eval(input);

        }
        catch(exception::eval_error& e)
        {
            std::cerr << e.pretty_print();
            assert(false);
        }


    }

    void eval_file(const std::string& filename)
    {
        try
        {
            chai.eval_file(filename);

        }
        catch(exception::eval_error& e)
        {
            std::cerr << e.pretty_print();
            //assert(false);
        }
    }



};

#else
class script_c::impl
{
public:
    void eval(const std::string& input)
    {
        std::cout << "eval: " << input << std::endl;
    }
    void eval_file(const std::string& filename)
    {
        std::cout << "eval file: " << filename << std::endl;
    }
        void register_callbacks(script_c * p) {}

};
#endif





script_c::script_c() : pimpl{std::make_unique<impl>()}
{
    eval_file("resources/preface.chai");
    eval_file("resources/script.chai");
    pimpl->register_callbacks(*this);
}

script_c::~script_c()
{

}

void script_c::eval(const std::string& input)
{
    return pimpl->eval(input);
}

void script_c::eval_file(const std::string& filename)
{
    return pimpl->eval_file(filename);
}

