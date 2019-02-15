//
// Created by fotyev on 2016-10-31.
//

#ifndef ZENGINE_SHADER_HPP
#define ZENGINE_SHADER_HPP

#include "math.hpp"

//#include <boost/noncopyable.hpp>
#include <boost/container/flat_map.hpp>

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
          my $dd = $d; $dd =~ s/x//;
          $s .= "template<> constexpr auto& gl${k}<$t, $dd> = gl::${k}${d}${types{$t}}v;
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

#if GLES_VERSION >= 3
          /*<
        generate_wrappers('Uniform', {'GLuint' => 'ui'}, [1..4]) .
        generate_wrappers('UniformMatrix', {'GLfloat' => 'f'}, [qw[2x3 3x2 2x4 4x2 3x4 4x3]]);
        %*/template<> constexpr auto& glUniform<GLuint, 1> = gl::Uniform1uiv;
          template<> constexpr auto& glUniform<GLuint, 2> = gl::Uniform2uiv;
          template<> constexpr auto& glUniform<GLuint, 3> = gl::Uniform3uiv;
          template<> constexpr auto& glUniform<GLuint, 4> = gl::Uniform4uiv;
          template<> constexpr auto& glUniformMatrix<GLfloat, 23> = gl::UniformMatrix2x3fv;
          template<> constexpr auto& glUniformMatrix<GLfloat, 32> = gl::UniformMatrix3x2fv;
          template<> constexpr auto& glUniformMatrix<GLfloat, 24> = gl::UniformMatrix2x4fv;
          template<> constexpr auto& glUniformMatrix<GLfloat, 42> = gl::UniformMatrix4x2fv;
          template<> constexpr auto& glUniformMatrix<GLfloat, 34> = gl::UniformMatrix3x4fv;
          template<> constexpr auto& glUniformMatrix<GLfloat, 43> = gl::UniformMatrix4x3fv;
          /*>*/
#endif

#if GLES_VERSION >= 3
        constexpr bool use_vao = true;
        constexpr auto& glGenVertexArrays = gl::GenVertexArrays;
        constexpr auto& glBindVertexArray = gl::BindVertexArray;
        constexpr auto& glDeleteVertexArrays = gl::DeleteVertexArrays;
#elif defined(GLES_EXTENSIONS)
        constexpr auto& glGenVertexArrays = gl::GenVertexArraysOES;
        constexpr auto& glBindVertexArray = gl::BindVertexArrayOES;
        constexpr auto& glDeleteVertexArrays = gl::DeleteVertexArraysOES;
        constexpr auto& use_vao = glBindVertexArray; // by casting to bool we will detect if extension is loaded at runtime
#else
        using voidfn = void (*)(...);
        voidfn glGenVertexArrays = nullptr, glBindVertexArray = nullptr, glDeleteVertexArrays = nullptr;
        constexpr bool use_vao = false;
#endif

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
    program(const char * file) { program p = from_file(file); swap(p); }

    // load both vertex and fragment shader from 1 file (contains preprocessor)
    static program from_file(const char * file);

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
    static GLuint create(program& prog, const char * name);
public:

    template <typename T, int D>
    inline void assign(const qvm::vec<T, D> * begin, const qvm::vec<T, D> * end) {
        detail::glUniform<T, D>(idx, end - begin, &begin->a[0]); GL_CHECK_ERROR();
    }
    template <typename T, int D>
    inline void operator = (const qvm::vec<T, D>& rhs) { assign(&rhs, &rhs + 1); }

    template <typename M>
    inline std::enable_if_t<qvm::is_mat<M>::value && qvm::mat_traits<M>::rows == qvm::mat_traits<M>::cols>
    operator = (const M &m) {
        using namespace qvm;
        constexpr auto D = qvm::mat_traits<M>::rows;
        using T = typename qvm::mat_traits<M>::scalar_type;
        mat<T, D, D> t = transposed(m);
        detail::glUniformMatrix<T, D>(idx, 1, false, &t.a[0][0]); GL_CHECK_ERROR();
    }

#if GLES_VERSION >= 3
// TODO: test
    template <typename T, int D1, int D2>
    inline std::enable_if_t<D1 != D2> assign(const qvm::mat<T, D1, D2> * begin, const qvm::mat<T, D1, D2> * end) {
        detail::glUniformMatrix<T, D1 * 10 + D2>(idx, end - begin, true, &begin->a[0][0]); GL_CHECK_ERROR();
    }
#endif
};

class attribute
{
    GLuint idx;
    friend class program;
private:
    explicit attribute(GLuint idx) : idx(idx) {}
    static GLuint create(program& prog, const char * name);
public:
    void setup(GLint size, GLenum type, bool normalized, GLsizei stride, std::ptrdiff_t offset);

    void disable();

    template <typename T, int D>
    inline void operator = (const qvm::vec<T, D>& rhs) { detail::glVertexAttrib<T, D>(idx, &rhs.a[0]); GL_CHECK_ERROR(); }
};

/* contains opengl vertex buffer object (array buffer), vertex array object, and stores an array of attributes
 * Storage requirements: (unsigned)size() - amount of elements stored, (const T *)data() - pointer to beginning
 */
template <typename T, class Storage>
class vertex_buffer : public Storage
{
private:
    GLuint vbo = 0;
    GLuint vao = 0;

    NONCOPYABLE(vertex_buffer)
public:
    template <typename ... Args>
    vertex_buffer(Args&&... args) : Storage(std::forward<Args...>(args)...)
    {
        gl::GenBuffers(1, &vbo);
        GL_CHECK_ERROR();
    }

    void upload(GLenum usage = GL_STATIC_DRAW) const
    {
        gl::BindBuffer(GL_ARRAY_BUFFER, vbo);
        gl::BufferData(GL_ARRAY_BUFFER, sizeof(T) * this->size(), this->data(), usage);
        GL_CHECK_ERROR();
    }

    void update(unsigned start, int count = -1) const
    {
        assume(start < this->size());
        unsigned cnt = static_cast<unsigned>((count != -1) ? count : (this->size() - start));
        assume(start + cnt <= this->size());

        gl::BindBuffer(GL_ARRAY_BUFFER, vbo);
        gl::BufferSubData(GL_ARRAY_BUFFER, sizeof(T) * start, sizeof(T) * count, this->data());
        GL_CHECK_ERROR();
    }

    /* You should only call draw with the same program, or make sure attribute locations in these programs match */
    void draw(program& p, GLenum mode = GL_TRIANGLES, unsigned skip = 0, int count = -1)
    {
        using namespace detail;

        assume(skip <= (unsigned)this->size());
        assume(count == -1 || unsigned(count) <= (this->size() - skip));
        unsigned cnt = static_cast<unsigned>((count != -1) ? count : (this->size() - skip));
        if(!cnt)
            return;

        p.bind();

        if(UNLIKELY(vao == 0))
        {
            if(use_vao)
            {
                glGenVertexArrays(1, &vao);
                GL_CHECK_ERROR();
                assume(vao);
                glBindVertexArray(vao);
            }
            gl::BindBuffer(GL_ARRAY_BUFFER, vbo);
            T::setup(p);
        }
        else
        {
            glBindVertexArray(vao);
        }
        gl::DrawArrays(mode, skip, cnt);
        GL_CHECK_ERROR();
        if(vao)
            glBindVertexArray(0);
        else
            T::disable(p);

    }

    ~vertex_buffer()
    {
        if(gl::initialized)
        {
            gl::DeleteBuffers(1, &vbo);
            GL_CHECK_ERROR();
            if(vao)
            {
                detail::glDeleteVertexArrays(1, &vao);
                GL_CHECK_ERROR();
            }
        }
    }
};

#include <vector>
template <typename T>
using vector_vertex_buffer = vertex_buffer<T, std::vector<T>>;



#define GET_UNIFORM(program,name) (program.check_binded(),program.get<uniform>(fnv1a::hash(const_string(#name)), #name))
#define GET_ATTRIBUTE(program,name) program.get<attribute>(fnv1a::hash(const_string(#name)), #name)


#endif //ZENGINE_SHADER_HPP
