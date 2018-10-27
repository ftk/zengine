//
// Created by fotyev on 2016-10-31.
//

#ifndef ZENGINE_SHADER_HPP
#define ZENGINE_SHADER_HPP

#include "math.hpp"

//#include <boost/noncopyable.hpp>
#include <boost/container/flat_map.hpp>
#include <set>

#include "util/hash.hpp"
#include "util/movable.hpp"

#include "opengl.hpp"

#include "util/assert.hpp"

namespace detail {
    namespace {
        template <typename T, int D>
        constexpr auto glUniform = nullptr;
        template <typename T, int D>
        constexpr auto glUniformMatrix = nullptr;
        template <typename T, int D>
        constexpr auto glVertexAttrib = nullptr;

        /*<
        sub generate_wrappers {
        my $k = shift;
        my %types = %{shift(@_)};
        my @dim = @{shift(@_)};
        my $s = '';
        for my $t (sort keys %types) {
         for my $d (@dim) {
          $s .= "template<> constexpr auto& gl${k}<$t, $d> = gl::${k}${d}${types{$t}}v;
          ";
         }
        }
        $s;
        }

        generate_wrappers('Uniform', {'GLfloat' => 'f', 'GLint' => 'i'}, [1..4]) .
        generate_wrappers('UniformMatrix', {'GLfloat' => 'f'}, [2..4]) .
        generate_wrappers('VertexAttrib', {'GLfloat' => 'f'}, [1..4]);
        %*/template<> constexpr auto& glUniform<GLfloat, 1> = gl::Uniform1fv;
          template<> constexpr auto& glUniform<GLfloat, 2> = gl::Uniform2fv;
          template<> constexpr auto& glUniform<GLfloat, 3> = gl::Uniform3fv;
          template<> constexpr auto& glUniform<GLfloat, 4> = gl::Uniform4fv;
          template<> constexpr auto& glUniform<GLint, 1> = gl::Uniform1iv;
          template<> constexpr auto& glUniform<GLint, 2> = gl::Uniform2iv;
          template<> constexpr auto& glUniform<GLint, 3> = gl::Uniform3iv;
          template<> constexpr auto& glUniform<GLint, 4> = gl::Uniform4iv;
          template<> constexpr auto& glUniformMatrix<GLfloat, 2> = gl::UniformMatrix2fv;
          template<> constexpr auto& glUniformMatrix<GLfloat, 3> = gl::UniformMatrix3fv;
          template<> constexpr auto& glUniformMatrix<GLfloat, 4> = gl::UniformMatrix4fv;
          template<> constexpr auto& glVertexAttrib<GLfloat, 1> = gl::VertexAttrib1fv;
          template<> constexpr auto& glVertexAttrib<GLfloat, 2> = gl::VertexAttrib2fv;
          template<> constexpr auto& glVertexAttrib<GLfloat, 3> = gl::VertexAttrib3fv;
          template<> constexpr auto& glVertexAttrib<GLfloat, 4> = gl::VertexAttrib4fv;
          /*>*/
    }
}

class shader
{
    GLuint glshader;

    NONCOPYABLE(shader)
public:
    GLuint get() const { return glshader; }

    shader(GLenum type, const char * source);

    static shader from_file(GLenum type, const char * file);

    ~shader();

};


class program
{
    GLuint prog = 0;

    typedef uint64_t hash_t;
    boost::container::flat_map<hash_t, GLuint> gl_indexes;

    NONCOPYABLE_BUT_SWAPPABLE(program, (prog)(gl_indexes))
public:
    GLuint get() const { return prog; }

    explicit program(const std::initializer_list<GLuint>& shaders);

    // load both vertex and fragment shader from 1 file (contains preprocessor)
    static program from_file(const char * file, std::set<std::string> params = std::set<std::string>());

    void bind() const
    {
        gl::UseProgram(prog);
        GL_CHECK_ERROR();
    }

    void check_binded() const // only for debugging forgotten program::bind();
    {
#ifndef NDEBUG
        GLint cur_prog;
        gl::GetIntegerv(GL_CURRENT_PROGRAM, &cur_prog);
        //GL_CHECK_ERROR();
        assert((GLuint)cur_prog == prog);
#endif
    }


    template <class Attributes>
    void draw(GLenum mode, unsigned first, unsigned count) //  array buffer should be binded before calling draw
    {

        check_binded(); // check that the program is used
        Attributes::setup(*this);
        gl::DrawArrays(mode, first, count);
        GL_CHECK_ERROR();
        Attributes::disable(*this);
    }

    ~program();

    bool has(hash_t hash) const { return !!gl_indexes.count(hash); }

    template <typename GLObj, typename... Args>
    int create(Args&&... args) const { return GLObj{GLObj::create(*this, std::forward<Args>(args)...)}; }


    template <typename GLObj, typename... Args>
    GLObj get(hash_t hash, Args&&... args)
    {
        auto it = gl_indexes.find(hash);
        if(it == gl_indexes.end())
        {
            // create new
            GLuint index = GLObj::create(*this, std::forward<Args>(args)...);
            gl_indexes[hash] = index;
            return GLObj{index};
        }
        return GLObj{it->second};
    }

    template <typename GLObj>
    GLObj find(hash_t hash)
    {
        auto it = gl_indexes.find(hash);
        assume(it != gl_indexes.end());
        return GLObj{it->second};
    }

    // lazy version
    template <typename GLObj, typename Callable>
    GLObj get_l(hash_t hash, const Callable& fn)
    {
        auto it = gl_indexes.find(hash);
        if(it == gl_indexes.end())
        {
            // create new
            GLuint index = GLObj::create(*this, fn());
            gl_indexes[hash] = index;
            return GLObj{index};
        }
        return GLObj{it->second};
    };

};

// GLObj :  static GLuint program(program& prog, ...), constructor(GLuint)

class uniform
{
    GLuint idx;
    friend class program;
private:
    explicit uniform(GLuint idx) : idx(idx) {}
    static GLuint create(program& prog, const char * name)
    {
        auto i = gl::GetUniformLocation(prog.get(), name);
        GL_CHECK_ERROR();
        assume(i >= 0);
        return static_cast<GLuint>(i);
    }
public:

    template <typename T, int D>
    inline void assign(const qvm::vec<T, D> * begin, const qvm::vec<T, D> * end) {
        detail::glUniform<T, D>(idx, end - begin, &begin->a[0]); GL_CHECK_ERROR();
    }
    template <typename T, int D>
    inline void operator = (const qvm::vec<T, D>& rhs) { assign(&rhs, &rhs + 1); }

    template <typename T, int D>
    inline void assign(const qvm::mat<T, D, D> * begin, const qvm::mat<T, D, D> * end) {
        detail::glUniformMatrix<T, D>(idx, end - begin, &begin->a[0][0]); GL_CHECK_ERROR();
    }
    template <typename T, int D>
    inline void operator = (const qvm::mat<T, D, D>& rhs) { assign(&rhs, &rhs + 1); }
};

class attribute
{
    GLuint idx;
    friend class program;
private:
    explicit attribute(GLuint idx) : idx(idx) {}
    static GLuint create(program& prog, const char * name)
    {
        auto i = gl::GetAttribLocation(prog.get(), name);
        if(i < 0)
            throw std::runtime_error{std::string("cant find attribute ") + name};
        GL_CHECK_ERROR();
        return GLuint(i);
    }
public:
    void setup(GLint size, GLenum type, bool normalized, GLsizei stride, std::ptrdiff_t offset)
    {
        gl::EnableVertexAttribArray(idx);
        gl::VertexAttribPointer(idx, size, type, (GLboolean)normalized, stride, (void *)offset);
        GL_CHECK_ERROR();
    }

    void disable()
    {
        gl::DisableVertexAttribArray(idx);
        GL_CHECK_ERROR();
    }

    template <typename T, int D>
    inline void operator = (const qvm::vec<T, D>& rhs) { detail::glVertexAttrib<T, D>(idx, &rhs.a[0]); GL_CHECK_ERROR(); }
};


// deprecated
class array_buffer
{
    GLuint idx;
    friend class program;
    friend class array_buf;
private:
    explicit array_buffer(GLuint idx) : idx(idx) {}
    static GLuint create(program& prog, const void * data, std::size_t size);
    template<class Container>
    static GLuint create(program& prog, const Container& container)
    {
        return create(prog, &container[0], container.size() * sizeof(container[0]));
    }


public:
    void bind()
    {
        gl::BindBuffer(GL_ARRAY_BUFFER, idx);
        GL_CHECK_ERROR();
    }

};

class array_buf
{
    GLuint buf = 0;

NONCOPYABLE_BUT_SWAPPABLE(array_buf, (buf))

public:
    array_buf(const void * data = nullptr, std::size_t size = 0, GLenum hint = GL_STATIC_DRAW);

    template<class Container>
    array_buf(const Container& container) : array_buf(container.data(), container.size() * sizeof(container[0]))
    {
    }

    /*template<class Container>
    array_buf& operator=(const Container& container)
    {
        gl::BufferData(GL_ARRAY_BUFFER, container.size() * sizeof(container[0]), container.data(), GL_STATIC_DRAW);
        GL_CHECK_ERROR();
        return *this;
    }*/


    ~array_buf();

    void bind()
    {
        gl::BindBuffer(GL_ARRAY_BUFFER, buf);
        GL_CHECK_ERROR();
    }

    static void unbind();

    void update(const void * data, std::size_t size, std::size_t offset = 0);
    void set(const void * data, std::size_t size, GLenum hint = GL_STATIC_DRAW);

};



#define GET_UNIFORM(program,name) (program.check_binded(),program.get<uniform>(fnv1a::hash(const_string(#name)), #name))
#define GET_ATTRIBUTE(program,name) program.get<attribute>(fnv1a::hash(const_string(#name)), #name)
//.setup(vlen, type_to_gl<stype>::value, false, sizeof(struc), offsetof(struc, name));


#endif //ZENGINE_SHADER_HPP
