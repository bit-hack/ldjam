#include <cstdint>
#include <memory>

#include "../external/glee/GLee.h"

#include "gl_draw.h"
#include "gl_shader.h"
#include "gl_texture.h"

#include "../framework_core/mat2.h"
#include "../framework_core/rect.h"
#include "../framework_core/string.h"
#include "../framework_core/vec2.h"
#include "../framework_core/vec3.h"
#include "../framework_draw/bitmap.h"

namespace {

// default, fallback vertex shader
const char default_vshader[] = R"(
#version 130
attribute vec4 pos;
attribute vec2 tex;
varying vec2 tex_out;
void main() {
    tex_out =  tex;
    gl_Position = pos;
})";

void gl_dumpShaderError(GLuint shader)
{
    GLint logSize = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
    if (logSize <= 0)
        return;
    std::unique_ptr<char[]> log(new char[logSize]);
    glGetShaderInfoLog(shader, logSize, nullptr, (GLchar*)log.get());
    log[logSize - 1] = '\0';
    printf("error: %s\n", log.get());
}

void gl_dumpProgramError(GLuint program)
{
    GLint logSize = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
    if (logSize <= 0)
        return;
    std::unique_ptr<char[]> log(new char[logSize]);
    glGetProgramInfoLog(program, logSize, nullptr, (GLchar*)log.get());
    log[logSize - 1] = '\0';
    printf("error: %s\n", log.get());
}
} // namespace {}

namespace tengu {
bool gl_shader_t::load(
    const buffer_t& vert,
    const buffer_t& frag)
{

    GLint status = -1;
    const GLchar* vd = vert.empty() ? (const GLchar*)default_vshader : (const GLchar*)vert.data();
    const GLuint vsobj = glCreateShader(GL_VERTEX_SHADER);
    if (vsobj == 0) {
        return false;
    }
    glShaderSource(vsobj, 1, &vd, nullptr);
    glCompileShader(vsobj);
    glGetShaderiv(vsobj, GL_COMPILE_STATUS, &status);
    if (!status) {
        gl_dumpShaderError(vsobj);
        return false;
    }
    const GLchar* fd = (const GLchar*)frag.data();
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

bool gl_shader_t::bind(const char* name, const gl_texture_t& val, const uint32_t slot)
{
    glActiveTexture(GL_TEXTURE0);
    GLint loc = glGetUniformLocation(program_, name);
    if (loc != GL_INVALID_VALUE) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(loc, slot);
        glUniform1i(loc, slot);
        return true;
    }
    return false;
}

bool gl_shader_t::bind(const char* name, const mat4f_t& val)
{
    const auto& itt = uniforms_.find(name);
    if (itt != uniforms_.end()) {
        glUniformMatrix4fv(0, 1, GL_FALSE, val.e.data());
        return true;
    }
    return false;
}

bool gl_shader_t::bind(const char* name, const mat2f_t& val)
{
    const auto& itt = uniforms_.find(name);
    if (itt != uniforms_.end()) {
        glUniformMatrix2fv(0, 1, GL_FALSE, val.e.data());
        return true;
    }
    return false;
}

bool gl_shader_t::bind(const char* name, const vec3f_t& val)
{
    const auto& itt = uniforms_.find(name);
    if (itt != uniforms_.end()) {
        glUniform3f(itt->second, val.x, val.y, val.z);
        return true;
    }
    return false;
}

bool gl_shader_t::bind(const char* name, const vec2f_t& val)
{
    const auto& itt = uniforms_.find(name);
    if (itt != uniforms_.end()) {
        glUniform2f(itt->second, val.x, val.y);
        return true;
    }
    return false;
}

bool gl_shader_t::bind(const char* name, float& val)
{
    const auto& itt = uniforms_.find(name);
    if (itt != uniforms_.end()) {
        glUniform1f(itt->second, val);
        return true;
    }
    return false;
}

void gl_shader_t::_inspect_shader()
{
    using namespace tengu;
    if (program_ == GL_INVALID_VALUE) {
        return;
    }
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

gl_shader_t::operator bool() const
{
    return program_ != GL_INVALID_VALUE;
}

gl_shader_t::gl_shader_t(gl_draw_t& draw)
    : draw_(draw)
    , program_(GL_INVALID_VALUE)
    , vert_(GL_INVALID_VALUE)
    , frag_(GL_INVALID_VALUE)
{
    draw_.shaders_.insert(this);
}

gl_shader_t::~gl_shader_t()
{
    release();
    draw_.shaders_.erase(this);
}

void gl_shader_t::release()
{
    // delete vertex shader
    if (vert_ != GL_INVALID_VALUE) {
        if (glIsShader(vert_)) {
            glDeleteShader(vert_);
        }
        vert_ = GL_INVALID_VALUE;
    }
    // delete fragment shader
    if (frag_ != GL_INVALID_VALUE) {
        if (glIsShader(frag_)) {
            glDeleteShader(frag_);
        }
        frag_ = GL_INVALID_VALUE;
    }
    // delete linked program
    if (program_ != GL_INVALID_VALUE) {
        if (glIsProgram(program_)) {
            glDeleteProgram(program_);
        }
        program_ = GL_INVALID_VALUE;
    }
}
} // tengu
