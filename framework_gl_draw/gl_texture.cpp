#include "../external/glee/GLee.h"

#include "gl_draw.h"
#include "gl_texture.h"

#include "../framework_core/rect.h"
#include "../framework_draw/bitmap.h"

namespace tengu {
gl_texture_t::gl_texture_t(gl_draw_t& draw)
    : draw_(draw)
    , id_(GL_INVALID_VALUE)
{
    draw_.textures_.insert(this);
}

gl_texture_t::~gl_texture_t()
{
    draw_.textures_.erase(this);
    release();
}

void gl_texture_t::release()
{
    GLuint name = id_;
    if (name != GL_INVALID_VALUE) {
        glDeleteTextures(1, &name);
    }
}

bool gl_texture_t::load(const bitmap_t& bmp)
{

    if (!bmp.valid()) {
        return false;
    }
    if (id_ == GL_INVALID_VALUE) {
        GLuint name = GL_INVALID_VALUE;
        glGenTextures(1, &name);
        id_ = name;
        glTexParameteri(GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_NEAREST);
    }
    if (id_ != GL_INVALID_VALUE) {
        glTexImage2D(GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            bmp.width(),
            bmp.height(),
            0,
            GL_BGRA,
            GL_UNSIGNED_INT_8_8_8_8_REV,
            bmp.data());
        const GLenum err = glGetError();
        return err == GL_NO_ERROR;
    }
    return false;
}

bool gl_texture_t::store(bitmap_t& out)
{
    //  glReadPixels(0, 0, out.width(), out.height(), GL_RGBA, GL_UNSIGNED_BYTE, out.data());
    return false;
}

bool gl_texture_t::copy_from(const rectf_t& dst)
{

    if (id_ == GL_INVALID_VALUE) {
        return false;
    }
    glBindTexture(GL_TEXTURE_2D, id_);
    glCopyTexImage2D(GL_TEXTURE_2D,
        0,
        GL_RGB8,
        dst.x0,
        dst.y0,
        dst.width(),
        dst.height(),
        0);
    return true;
}

gl_texture_t::operator bool() const
{
    return id_ != GL_INVALID_VALUE;
}
} // namespace tengu
