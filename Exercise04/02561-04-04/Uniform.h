#include "Angel.h"
#include <forward_list>

template <class T>
class _UniformHandler;

template<>
class _UniformHandler<mat4>
{
public:
    inline void SetUniform(const mat4& Data, const GLint &Uniform_in) const
    {
        glUniformMatrix4fv(Uniform_in, 1, GL_FALSE, Data);
    }
    inline void SetUniform_DSA(const mat4& Data, const GLuint &Program_in, const GLint &Uniform_in) const
    {
        glProgramUniformMatrix4fvEXT(Program_in, Uniform_in, 1, GL_FALSE, Data);
    }
};
template <>
class _UniformHandler<vec4>
{
public:
    inline void SetUniform(const vec4& Data, const GLint &Uniform_in) const
    {
        glUniform4fv(Uniform_in, 1, Data);
    }
    inline void SetUniform_DSA(const vec4& Data, const GLuint &Program_in, const GLint &Uniform_in) const
    {
        glProgramUniform4fvEXT(Program_in, Uniform_in, 1, Data);
    }
};

template <>
class _UniformHandler<vec3>
{
public:
    inline void SetUniform(const vec3& Data, const GLint &Uniform_in) const
    {
        glUniform3fv(Uniform_in, 1, Data);
    }
    inline void SetUniform_DSA(const vec3& Data, const GLuint &Program_in, const GLint &Uniform_in) const
    {
        glProgramUniform3fvEXT(Program_in, Uniform_in, 1, Data);
    }
};

template <>
class _UniformHandler<vec2>
{
public:
    inline void SetUniform(const vec2& Data, const GLint &Uniform_in) const
    {
        glUniform2fv(Uniform_in, 1, Data);
    }
    inline void SetUniform_DSA(const vec2& Data, const GLuint &Program_in, const GLint &Uniform_in) const
    {
        glProgramUniform2fvEXT(Program_in, Uniform_in, 1, Data);
    }
};

template <>
class _UniformHandler<float>
{
public:
    inline void SetUniform(const float& Data, const GLint &Uniform_in) const
    {
        glUniform1fv(Uniform_in, 1, &Data);
    }
    inline void SetUniform_DSA(const float& Data, const GLuint &Program_in, const GLint &Uniform_in) const
    {
        glProgramUniform1fvEXT(Program_in, Uniform_in, 1, &Data);
    }
};

template <>
class _UniformHandler<int>
{
public:
    inline void SetUniform(const int& Data, const GLint &Uniform_in) const
    {
        glUniform1iv(Uniform_in, 1, &Data);
    }
    inline void SetUniform_DSA(const int& Data, const GLuint &Program_in, const GLint &Uniform_in) const
    {
        glProgramUniform1ivEXT(Program_in, Uniform_in, 1, &Data);
    }
};

template <class T>
class _Uniform
{
protected:
    // Each program-uniform pair is stored in a forward_list
    std::forward_list<std::pair<GLuint,GLint>> Uniforms;
    _UniformHandler<T> UniformHandler;

public:
    _Uniform() {}
    _Uniform(const _Uniform& cp): Uniforms(cp.Uniforms) {}
    _Uniform(_Uniform&& mv): Uniforms(std::move(mv.Uniforms)) {}
    _Uniform& operator=(const _Uniform& cp)
    {
        Uniforms = cp.Uniforms;
        return *this;
    }
    _Uniform& operator=(_Uniform&& mv)
    {
        std::swap(Uniforms, mv.Uniforms);
        return *this;
    }
    ~_Uniform() {}

    // Gets a handle to the OpenGL uniform, given the program and the uniform name
    int Register(GLuint Program, const char *Name_in)
    {
        GLint Location;
        if((Location = glGetUniformLocation(Program, Name_in)) == -1) return 1;
        Uniforms.push_front(std::pair<GLuint, GLint>(Program, Location));
        return 0;
    }

    // Clears out the uniform handle list...haven't used it at all
    void ClearUniforms()
    {
        Uniforms.clear();
    }

    // Sets all stored uniforms to Data_in
    void SetData(const T &Data_in)
    {
        // Can use GL_EXT_direct_state_access extension or plain OpenGL
        // Guess which one I like better
        #ifdef DIRECT_STATE_ACCESS
            for(auto &i : Uniforms)
            {
                UniformHandler.SetUniform_DSA(Data_in, i.first, i.second);
            }
        #else
            GLuint Initial;
            glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)(&Initial));
            GLuint Previous = Initial;
            for(auto &i : Uniforms)
            {
                if(i.first != Previous)
                {
                    glUseProgram(i.first);
                    Previous = i.first;
                }
                UniformHandler.SetUniform(Data_in, i.second);
            }
            if(Initial != Previous) glUseProgram(Initial);
        #endif
    }

};