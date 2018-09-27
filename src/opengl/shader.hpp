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

    /*<
     sub generate_setters {
     my $setter = shift;
     my %types = %{shift(@_)};
     my $s = '';
     for my $t (sort keys %types) {
     for my $d (1..4) {
     my $params = '';
     #$params .= ", qvm::vec_traits<T>::template read_element<$_>(rhs)" for (0 .. $d-1);
     $params .= ", qvm::A<$_>(rhs)" for (0 .. $d-1);
     $s .= qq%
     // vec $t $d
     template <typename T>
     std::enable_if_t<qvm::is_vec<T>::value && qvm::vec_traits<T>::dim == $d &&
     std::is_same<$t, typename qvm::vec_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        ${setter}${d}${types{$t}}(idx $params); GL_CHECK_ERROR();
     }
     %;
     }
     }

     return $s;
     }

     generate_setters('gl::Uniform', { 'GLfloat' => 'f', 'GLint' => 'i' });
     %*/
     // vec GLfloat 1
     template <typename T>
     std::enable_if_t<qvm::is_vec<T>::value && qvm::vec_traits<T>::dim == 1 &&
     std::is_same<GLfloat, typename qvm::vec_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        gl::Uniform1f(idx , qvm::A<0>(rhs)); GL_CHECK_ERROR();
     }
     
     // vec GLfloat 2
     template <typename T>
     std::enable_if_t<qvm::is_vec<T>::value && qvm::vec_traits<T>::dim == 2 &&
     std::is_same<GLfloat, typename qvm::vec_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        gl::Uniform2f(idx , qvm::A<0>(rhs), qvm::A<1>(rhs)); GL_CHECK_ERROR();
     }
     
     // vec GLfloat 3
     template <typename T>
     std::enable_if_t<qvm::is_vec<T>::value && qvm::vec_traits<T>::dim == 3 &&
     std::is_same<GLfloat, typename qvm::vec_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        gl::Uniform3f(idx , qvm::A<0>(rhs), qvm::A<1>(rhs), qvm::A<2>(rhs)); GL_CHECK_ERROR();
     }
     
     // vec GLfloat 4
     template <typename T>
     std::enable_if_t<qvm::is_vec<T>::value && qvm::vec_traits<T>::dim == 4 &&
     std::is_same<GLfloat, typename qvm::vec_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        gl::Uniform4f(idx , qvm::A<0>(rhs), qvm::A<1>(rhs), qvm::A<2>(rhs), qvm::A<3>(rhs)); GL_CHECK_ERROR();
     }
     
     // vec GLint 1
     template <typename T>
     std::enable_if_t<qvm::is_vec<T>::value && qvm::vec_traits<T>::dim == 1 &&
     std::is_same<GLint, typename qvm::vec_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        gl::Uniform1i(idx , qvm::A<0>(rhs)); GL_CHECK_ERROR();
     }
     
     // vec GLint 2
     template <typename T>
     std::enable_if_t<qvm::is_vec<T>::value && qvm::vec_traits<T>::dim == 2 &&
     std::is_same<GLint, typename qvm::vec_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        gl::Uniform2i(idx , qvm::A<0>(rhs), qvm::A<1>(rhs)); GL_CHECK_ERROR();
     }
     
     // vec GLint 3
     template <typename T>
     std::enable_if_t<qvm::is_vec<T>::value && qvm::vec_traits<T>::dim == 3 &&
     std::is_same<GLint, typename qvm::vec_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        gl::Uniform3i(idx , qvm::A<0>(rhs), qvm::A<1>(rhs), qvm::A<2>(rhs)); GL_CHECK_ERROR();
     }
     
     // vec GLint 4
     template <typename T>
     std::enable_if_t<qvm::is_vec<T>::value && qvm::vec_traits<T>::dim == 4 &&
     std::is_same<GLint, typename qvm::vec_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        gl::Uniform4i(idx , qvm::A<0>(rhs), qvm::A<1>(rhs), qvm::A<2>(rhs), qvm::A<3>(rhs)); GL_CHECK_ERROR();
     }
     /*>*/

    // faster overloads
    // mat4
    void
    operator = (const qvm::mat4& rhs)
    {
        gl::UniformMatrix4fv(idx, 1, true, &rhs.a[0][0]);
        GL_CHECK_ERROR();
    }
    // mat3
    void
    operator = (const qvm::mat3& rhs)
    {
        gl::UniformMatrix3fv(idx, 1, true, &rhs.a[0][0]);
        GL_CHECK_ERROR();
    }

    // generic qvm overloads
    /*<
     my $s = '';
     for my $d (2..4) {
     my $mat = '';
     for my $col (0..$d-1) { for my $row (0..$d-1) {
         $mat .= "qvm::A<$row,$col>(rhs),";
     }}

     $s .= qq%
     // mat GLfloat $d
     template <typename T>
     std::enable_if_t<qvm::is_mat<T>::value &&
        qvm::mat_traits<T>::rows == $d && qvm::mat_traits<T>::cols == $d &&
        std::is_same<GLfloat, typename qvm::mat_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        const GLfloat mat[$d * $d] = {$mat};
        gl::UniformMatrix${d}fv(idx, 1, GL_FALSE, mat); GL_CHECK_ERROR();
     }
 %;
 }

 return $s;
 %*/
     // mat GLfloat 2
     template <typename T>
     std::enable_if_t<qvm::is_mat<T>::value &&
        qvm::mat_traits<T>::rows == 2 && qvm::mat_traits<T>::cols == 2 &&
        std::is_same<GLfloat, typename qvm::mat_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        const GLfloat mat[2 * 2] = {qvm::A<0,0>(rhs),qvm::A<1,0>(rhs),qvm::A<0,1>(rhs),qvm::A<1,1>(rhs),};
        gl::UniformMatrix2fv(idx, 1, GL_FALSE, mat); GL_CHECK_ERROR();
     }
 
     // mat GLfloat 3
     template <typename T>
     std::enable_if_t<qvm::is_mat<T>::value &&
        qvm::mat_traits<T>::rows == 3 && qvm::mat_traits<T>::cols == 3 &&
        std::is_same<GLfloat, typename qvm::mat_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        const GLfloat mat[3 * 3] = {qvm::A<0,0>(rhs),qvm::A<1,0>(rhs),qvm::A<2,0>(rhs),qvm::A<0,1>(rhs),qvm::A<1,1>(rhs),qvm::A<2,1>(rhs),qvm::A<0,2>(rhs),qvm::A<1,2>(rhs),qvm::A<2,2>(rhs),};
        gl::UniformMatrix3fv(idx, 1, GL_FALSE, mat); GL_CHECK_ERROR();
     }
 
     // mat GLfloat 4
     template <typename T>
     std::enable_if_t<qvm::is_mat<T>::value &&
        qvm::mat_traits<T>::rows == 4 && qvm::mat_traits<T>::cols == 4 &&
        std::is_same<GLfloat, typename qvm::mat_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        const GLfloat mat[4 * 4] = {qvm::A<0,0>(rhs),qvm::A<1,0>(rhs),qvm::A<2,0>(rhs),qvm::A<3,0>(rhs),qvm::A<0,1>(rhs),qvm::A<1,1>(rhs),qvm::A<2,1>(rhs),qvm::A<3,1>(rhs),qvm::A<0,2>(rhs),qvm::A<1,2>(rhs),qvm::A<2,2>(rhs),qvm::A<3,2>(rhs),qvm::A<0,3>(rhs),qvm::A<1,3>(rhs),qvm::A<2,3>(rhs),qvm::A<3,3>(rhs),};
        gl::UniformMatrix4fv(idx, 1, GL_FALSE, mat); GL_CHECK_ERROR();
     }
 /*>*/

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

    /*<
     generate_setters('gl::VertexAttrib', { 'GLfloat' => 'f' });
     %*/
     // vec GLfloat 1
     template <typename T>
     std::enable_if_t<qvm::is_vec<T>::value && qvm::vec_traits<T>::dim == 1 &&
     std::is_same<GLfloat, typename qvm::vec_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        gl::VertexAttrib1f(idx , qvm::A<0>(rhs)); GL_CHECK_ERROR();
     }
     
     // vec GLfloat 2
     template <typename T>
     std::enable_if_t<qvm::is_vec<T>::value && qvm::vec_traits<T>::dim == 2 &&
     std::is_same<GLfloat, typename qvm::vec_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        gl::VertexAttrib2f(idx , qvm::A<0>(rhs), qvm::A<1>(rhs)); GL_CHECK_ERROR();
     }
     
     // vec GLfloat 3
     template <typename T>
     std::enable_if_t<qvm::is_vec<T>::value && qvm::vec_traits<T>::dim == 3 &&
     std::is_same<GLfloat, typename qvm::vec_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        gl::VertexAttrib3f(idx , qvm::A<0>(rhs), qvm::A<1>(rhs), qvm::A<2>(rhs)); GL_CHECK_ERROR();
     }
     
     // vec GLfloat 4
     template <typename T>
     std::enable_if_t<qvm::is_vec<T>::value && qvm::vec_traits<T>::dim == 4 &&
     std::is_same<GLfloat, typename qvm::vec_traits<T>::scalar_type>::value>
     inline operator = (const T& rhs) {
        gl::VertexAttrib4f(idx , qvm::A<0>(rhs), qvm::A<1>(rhs), qvm::A<2>(rhs), qvm::A<3>(rhs)); GL_CHECK_ERROR();
     }
     /*>*/
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
