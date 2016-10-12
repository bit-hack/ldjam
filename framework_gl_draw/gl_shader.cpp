#include <memory>
#include "../external/glee/GLee.h"
#include "gl_shader.h"
#include "gl_texture.h"
#include "../framework_draw/bitmap.h"
#include "../framework_core/string.h"
#include "../framework_core/rect.h"
#include "../framework_core/mat2.h"
#include "../framework_core/vec2.h"
#include "../framework_core/vec3.h"

namespace {
void gl_dumpShaderError(GLuint shader) {
    GLint logSize = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
    if (logSize <= 0)
        return;
    std::unique_ptr<char[]> log(new char[logSize]);
    glGetShaderInfoLog(shader, logSize, nullptr, (GLchar *) log.get());
    log[logSize - 1] = '\0';
    printf("error: %s\n", log.get());
}

void gl_dumpProgramError(GLuint program) {
    GLint logSize = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
    if (logSize <= 0)
        return;
    std::unique_ptr<char[]> log(new char[logSize]);
    glGetProgramInfoLog(program, logSize, nullptr, (GLchar *) log.get());
    log[logSize - 1] = '\0';
    printf("error: %s\n", log.get());
}
} // namespace {}

namespace tengu {
bool gl_shader_t::load(
        const buffer_t &vert,
        const buffer_t &frag) {

    GLint status = -1;

    const GLchar * vd = (const GLchar *)vert.data();
    const GLuint vsobj = glCreateShader(GL_VERTEX_SHADER);
    if (vsobj == 0) {
        return false;
    }
    glShaderSource(vsobj, 1, &vd, nullptr);
    glCompileShader(vsobj );
    glGetShaderiv(vsobj, GL_COMPILE_STATUS, &status);
    if (!status) {
        gl_dumpShaderError(vsobj);
        return false;
    }
    const GLchar * fd = (const GLchar *)frag.data();
    const GLuint fsobj = glCreateShader(GL_FRAGMENT_SHADER);
    if (fsobj == 0) {
        return false;
    }
    glShaderSource(fsobj, 1, &fd, nullptr);
    glCompileShader(fsobj);
    glGetShaderiv(fsobj, GL_COMPILE_STATUS, &status);
    if (!status) {
        gl_dumpShaderError(fsobj);
        return false;
    }
    const GLuint program = glCreateProgram();
    if (program == 0) {
        return false;
    }
    glAttachShader(program, vsobj);
    glAttachShader(program, fsobj);
    glLinkProgram(program);
    // check if we linked successfully
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        gl_dumpProgramError(program);
        glDeleteProgram(program);
        return false;
    }
    // write out these objects
    program_ = program;
    vert_ = vsobj;
    frag_ = fsobj;
    // map out the uniforms/attribs present in this program
    _inspect_shader();
    // success
    return true;
}

bool gl_shader_t::bind(const char *name, gl_texture_t &val) {

    // how to specify different slots?

    const GLuint slot = 0;
    glActiveTexture(GL_TEXTURE0);
    GLint loc = glGetUniformLocation(program_, name);
    glUniform1i(loc, slot);
    glBindTexture(loc, slot);
    return true;
}

bool gl_shader_t::bind(const char *name, const mat4f_t &val) {
    return false;
}

bool gl_shader_t::bind(const char *name, const mat2f_t &val) {
    return false;
}

bool gl_shader_t::bind(const char *name, const vec3f_t &val) {
    return false;
}

bool gl_shader_t::bind(const char *name, const vec2f_t &val) {
    return false;
}

bool gl_shader_t::bind(const char *name, float &val) {
    return false;
}

void gl_shader_t::activate() {
    assert(program_);
    glUseProgram(program_);
}

void gl_shader_t::_inspect_shader() {
    using namespace tengu;

    glUseProgram(program_);
    // enumerate uniforms
    {
        // inspect uniforms
        GLint count = 0;
        glGetProgramiv(program_, GL_ACTIVE_UNIFORMS, &count);
        GLint max_len = 0;
        glGetProgramiv(program_, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_len);
        // buffer space for uniform name
        std::unique_ptr<char[]> buffer(new char[max_len]);
        // loop over uniform count
        for (GLuint i = 0; i < count; ++i) {
            GLsizei written = 0;
            GLint size = 0;
            GLenum type = 0;
            glGetActiveUniform(program_, i, max_len, &written, &size, &type, buffer.get());
            // get the uniform location
            const GLint loc = glGetUniformLocation(program_, buffer.get());
            uniforms_[string_t(buffer.get())] = loc;
        }
    }
    // enumerate attributes
    {
        // inspect attributes
        GLint count = 0;
        glGetProgramiv(program_, GL_ACTIVE_ATTRIBUTES, &count);
        GLint max_len = 0;
        glGetProgramiv(program_, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_len);
        // buffer space for attribute name
        std::unique_ptr<char[]> buffer(new char[max_len]);
        // loop over attributes
        for (GLuint i = 0; i < count; ++i) {
            GLsizei written = 0;
            GLint size = 0;
            GLenum type = 0;
            glGetActiveAttrib(program_, i, max_len, &written, &size, &type, buffer.get());
            // get the attribute location
            const GLint loc = glGetAttribLocation(program_, buffer.get());
            attribs_[string_t(buffer.get())] = loc;
        }
    }
}
}

#if 0
u32 cShader::getUniformId( char *name ) {
    return glGetUniformLocation( program, name );
    u32 error = glGetError( );
}

void cShader::setUniform( u32 id, const mat4 &data ) {
    glUniformMatrix4fv( id, 1, GL_FALSE, data.e );
    u32 error = glGetError( );
}

void cShader::setUniform( u32 id, const vec3 &data ) {
}

void cShader::setUniform( u32 id, const int &data ) {
    glUniform1i( id, data );
    u32 error = glGetError( );
}
#endif
