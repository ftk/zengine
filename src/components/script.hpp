//
// Created by fotyev on 2017-01-25.
//

#ifndef ZENGINE_SCRIPT_HPP
#define ZENGINE_SCRIPT_HPP

#include <memory>
#include <string>
#include <functional>

namespace chaiscript { class ChaiScript_Basic; }

template <typename R, typename... Args>
struct script_callback;

template <typename R, typename... Args>
struct script_callback<R (Args...)>
{
	std::function<R (Args...)> f;
	R operator () (Args... args);
	void init(const char * name, chaiscript::ChaiScript_Basic& engine) noexcept;
};



//=- #register_component(class=>'script_c', name=>'script', priority=>100);
class script_c
{
    class impl;
    std::unique_ptr<impl> pimpl;
public:
    script_c();
    ~script_c();

    void eval(const std::string& input);
    void eval_file(const std::string& filename);

    // callbacks

    /*<
       join "\n\t", map { "script_callback<$_->{type}> $_->{name};" } dispatch('callbacks');
     %*//*>*/

};


#endif //ZENGINE_SCRIPT_HPP
