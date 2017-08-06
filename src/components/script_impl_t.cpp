//
// Created by fotyev on 2017-02-27.
//
#ifndef NO_SCRIPTING
#include "script.hpp"
#include <chaiscript/chaiscript.hpp>
#include "util/assert.hpp"

#include <vector>


using namespace chaiscript;

ModulePtr script_register_types(ModulePtr m)
{
    auto& chai = *m;
    bootstrap::standard_library::vector_type<std::vector<uint64_t> >("UIntVector", chai);
    bootstrap::standard_library::vector_type<std::vector<std::string> >("StringVector", chai);
    return m;
}

ModulePtr create_chaiscript_stdlib()
{
    return Std_Lib::library();
}

std::unique_ptr<chaiscript::parser::ChaiScript_Parser_Base> create_chaiscript_parser()
{
    return std::make_unique<chaiscript::parser::ChaiScript_Parser<chaiscript::eval::Noop_Tracer, chaiscript::optimizer::Optimizer_Default>>();
}

#endif
