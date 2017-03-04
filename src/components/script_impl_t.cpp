//
// Created by fotyev on 2017-02-27.
//

#include "script.hpp"
#define CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_NO_THREADS_WARNING
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
