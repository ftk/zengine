//
// Created by fotyev on 2017-02-27.
//
#ifndef NO_SCRIPTING
#include "script.hpp"
#include <chaiscript/chaiscript.hpp>
#include "util/assert.hpp"

#include <vector>
#include "opengl/math.hpp"

using namespace chaiscript;

ModulePtr script_register_types(ModulePtr m)
{
    auto& chai = *m;
    bootstrap::standard_library::vector_type<std::vector<uint64_t> >("UIntVector", chai);
    bootstrap::standard_library::vector_type<std::vector<std::string> >("StringVector", chai);

    using namespace qvm;
    /*<
     my $s = '';
     my @p = qw[x y z w];
     for my $d (1..4) {
      $s .= qq[\n\tchai.add(user_type<vec${d}>(), "vec${d}");];
      my @pd = @p[0..$d-1];
      my $ts = join ', ', map {"float $_"} @pd;
      my $ps = join ', ', @pd;
      $s .= qq[\n\tchai.add(fun([]($ts) -> vec${d} { return vec${d}{$ps}; }), "vec${d}");];
      $s .= qq[\n\tchai.add(fun([](vec${d}& v) -> float& { return \u$_(v); }), "$_");] for @pd;
      $s .= qq[\n\tchai.add(fun([](vec${d}& v, unsigned int i) -> float& { assume(i < $d); return v.a[i]; }), "a");];
     }
     $s;
     %*/
	chai.add(user_type<vec1>(), "vec1");
	chai.add(fun([](float x) -> vec1 { return vec1{x}; }), "vec1");
	chai.add(fun([](vec1& v) -> float& { return X(v); }), "x");
	chai.add(fun([](vec1& v, unsigned int i) -> float& { assume(i < 1); return v.a[i]; }), "a");
	chai.add(user_type<vec2>(), "vec2");
	chai.add(fun([](float x, float y) -> vec2 { return vec2{x, y}; }), "vec2");
	chai.add(fun([](vec2& v) -> float& { return X(v); }), "x");
	chai.add(fun([](vec2& v) -> float& { return Y(v); }), "y");
	chai.add(fun([](vec2& v, unsigned int i) -> float& { assume(i < 2); return v.a[i]; }), "a");
	chai.add(user_type<vec3>(), "vec3");
	chai.add(fun([](float x, float y, float z) -> vec3 { return vec3{x, y, z}; }), "vec3");
	chai.add(fun([](vec3& v) -> float& { return X(v); }), "x");
	chai.add(fun([](vec3& v) -> float& { return Y(v); }), "y");
	chai.add(fun([](vec3& v) -> float& { return Z(v); }), "z");
	chai.add(fun([](vec3& v, unsigned int i) -> float& { assume(i < 3); return v.a[i]; }), "a");
	chai.add(user_type<vec4>(), "vec4");
	chai.add(fun([](float x, float y, float z, float w) -> vec4 { return vec4{x, y, z, w}; }), "vec4");
	chai.add(fun([](vec4& v) -> float& { return X(v); }), "x");
	chai.add(fun([](vec4& v) -> float& { return Y(v); }), "y");
	chai.add(fun([](vec4& v) -> float& { return Z(v); }), "z");
	chai.add(fun([](vec4& v) -> float& { return W(v); }), "w");
	chai.add(fun([](vec4& v, unsigned int i) -> float& { assume(i < 4); return v.a[i]; }), "a");/*>*/
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
