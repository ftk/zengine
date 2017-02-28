//
// Created by fotyev on 2017-01-25.
//

#include "script.hpp"
#include <iostream>



#ifndef CHAISCRIPT
#define CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_NO_THREADS_WARNING

#include <chaiscript/chaiscript.hpp>

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

/*< join "\n", map { qq%template struct script_callback<$_->{type}>;% } dispatch('callbacks');;
        %*/template struct script_callback<void ()>;
template struct script_callback<void (int)>;/*>*/


ModulePtr script_register_bindings(ModulePtr m = std::make_shared<Module>());
ModulePtr script_register_types(ModulePtr m = std::make_shared<Module>());

class script_c::impl
{
public:
    ChaiScript chai;
    impl()
    {
        register_functions();
    }

    void register_functions()
    {
        chai.add(fun([](const char * str) { return fnv1a::hash(str); }), "hash");

        chai.add(
                script_register_bindings(
                        script_register_types()
                )
        );
    }

    void register_callbacks(script_c& p)
    {
        /*< join "\n\t\t", map { qq%try{p.$_->{name} = {chai.eval<std::function<$_->{type}> >("$_->{name}")};}catch(exception::eval_error&){}% } dispatch('callbacks');
        %*/try{p.on_init = {chai.eval<std::function<void ()> >("on_init")};}catch(exception::eval_error&){}
		try{p.on_option_selected = {chai.eval<std::function<void (int)> >("on_option_selected")};}catch(exception::eval_error&){}/*>*/
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

