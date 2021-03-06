//
// Created by fotyev on 2016-11-12.
//
#include "shader.hpp"
#include "opengl.hpp"
#include <fstream>
#include <vector>
#include <set>

#include "util/log.hpp"

shader::shader(GLenum type, const char * source)
{
    // Create the shader object
    glshader = gl::CreateShader(type);
    GL_CHECK_ERROR();
    assert(glshader != 0);

    // Load the shader source
    gl::ShaderSource(glshader, 1, &source, NULL);

    // Compile the shader
    gl::CompileShader(glshader);

    // Check the compile status
    GLint compiled;
    gl::GetShaderiv(glshader, GL_COMPILE_STATUS, &compiled);

    if(!compiled)
    {
        GLint infoLen = 0;

        gl::GetShaderiv(glshader, GL_INFO_LOG_LENGTH, &infoLen);

        std::string info;
        if(infoLen > 0)
        {
            info.resize(infoLen);

            gl::GetShaderInfoLog(glshader, infoLen, NULL, &info[0]);
        }
        throw std::runtime_error{std::move(info)};
    }
    GL_CHECK_ERROR();

}

shader shader::from_file(GLenum type, const char * file)
{
    std::ifstream t(file);
    if(!t)
        throw std::runtime_error{"cannot open file " + std::string(file)};
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    return {type, str.c_str()};
}

shader::~shader()
{
    gl::DeleteShader(glshader);
    GL_CHECK_ERROR();
}

program::program(const std::initializer_list <GLuint>& shaders)
{
    prog = gl::CreateProgram();
    GL_CHECK_ERROR();
    assert(prog != 0);

    for(auto shader : shaders)
    {
        gl::AttachShader(prog, shader);
        GL_CHECK_ERROR();
    }
    gl::LinkProgram(prog);
    GL_CHECK_ERROR();


    GLint status;
    gl::GetProgramiv(prog, GL_LINK_STATUS, &status);
    if(!status)
    {
        GLint infoLen;
        gl::GetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLen);

        std::string info;
        if(infoLen > 0)
        {
            info.resize(infoLen);

            gl::GetProgramInfoLog(prog, infoLen, NULL, &info[0]);
        }
        throw std::runtime_error{std::move(info)};
    }

    for(auto shader : shaders)
    {
        gl::DetachShader(prog, shader);
        GL_CHECK_ERROR();
    }
    LOGGER(debug, "GL program", prog, "linked successfully");
}

program::~program()
{
    if(gl::initialized && prog)
    {
        //GL_CHECK_ERROR();
        gl::DeleteProgram(prog);
        GL_CHECK_ERROR();
    }
}

program program::from_file(const char * file)
{
    std::set<std::string> params;

    LOGGER(debug, "Loading GL program from", file);

    std::ifstream f(file);
    if(!f)
        throw std::runtime_error{"cannot open file " + std::string(file)};



    auto preprocess = [&f,&params]() {
        std::vector<bool> stack;
        stack.push_back(true);

        std::string str, source;
        while(std::getline(f, str))
        {
            if(str.compare(0, 4, "//@{") == 0) // ifdef
            {
                auto param = str.substr(4);
                if(params.count(param)/* && params[param]*/)
                    stack.push_back(stack.back());
                else
                    stack.push_back(false);
            }
            else if(str.compare(0, 4, "//@=") == 0) // else
            {
                assert(stack.size() >= 2);
                bool x = stack.back();
                stack.pop_back();
                stack.push_back(stack.back() ^ x);
            }
            else if(str.compare(0, 4, "//@}") == 0) // endif
            {
                stack.pop_back();
                assert(!stack.empty());
            }
            else if(stack.back())
                source += std::move(str);

            source += '\n';
        }
        return source;
    };

    // vertex
    params.insert("vertex");
//    params["vertex"] = true;
//    params["fragment"] = false;

    auto vert_src = preprocess();
    shader vert {GL_VERTEX_SHADER, vert_src.c_str()};


    // fragment

    f.clear();
    f.seekg(0, std::ios_base::beg);
    //f = std::ifstream(file);

    params.erase("vertex");
    params.insert("fragment");

//    params["vertex"] = false;
//    params["fragment"] = true;

    auto frag_src = preprocess();
    shader frag {GL_FRAGMENT_SHADER, frag_src.c_str()};



    return program{vert.get(), frag.get()};
}

GLuint uniform::create(program& prog, const char * name)
{
    auto i = gl::GetUniformLocation(prog.get(), name);
    GL_CHECK_ERROR();
    assume(i >= 0);
    return static_cast<GLuint>(i);
}

GLuint attribute::create(program& prog, const char * name)
{
    auto i = gl::GetAttribLocation(prog.get(), name);
    if(i < 0)
        throw std::runtime_error{std::string("cant find attribute ") + name};
    GL_CHECK_ERROR();
    return GLuint(i);
}

void attribute::setup(GLint size, GLenum type, bool normalized, GLsizei stride, std::ptrdiff_t offset)
{
    gl::EnableVertexAttribArray(idx);
    gl::VertexAttribPointer(idx, size, type, (GLboolean)normalized, stride, (void *)offset);
    GL_CHECK_ERROR();
}

void attribute::disable()
{
    gl::DisableVertexAttribArray(idx);
    GL_CHECK_ERROR();
}

/*<
sub define_attributes {
my $struct = shift;
my %attrs = %{shift()};

my $s = "
#pragma pack(push,1)
struct $struct
{";

for my $at (sort keys %attrs) {
my $type = $attrs{$at}->[0];
$s .= "
$type \t$at;";
}

# setup function
$s .= q%
static void setup(program& p) {%;
for my $at (sort keys %attrs) {
@_ = @{$attrs{$at}};
my $type = shift;
my $subtype = shift // $type;
my $packed = shift // 'false';
my $dim = shift // "sizeof($type)/sizeof($subtype)";

$s .= qq%
    GET_ATTRIBUTE(p,$at).setup($dim, type_to_gl<$subtype>::value, $packed, sizeof($struct), offsetof($struct, $at));%;

}
$s .= q%
}
static void disable(program& p) {%;
for my $at (sort keys %attrs) {
 $s .= qq%
    GET_ATTRIBUTE(p,$at).disable();%;
}
$s .= '
}
};
#pragma pack(pop)
';
}
>*/