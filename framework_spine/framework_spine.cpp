#include "framework_spine.h"
#include "../framework_core/buffer.h"
#include "../external/stb_image/stb_image.h"
#include "../framework_draw/draw.h"

// using info from http://esotericsoftware.com/spine-c

extern "C" {

void _spAtlasPage_createTexture(spAtlasPage* self, const char* path)
{
    int w, h, chans;
    stbi_uc* pix = stbi_load(path, &w, &h, &chans, 4);

    if (!pix) {
      w = 1;
      h = 1;
      chans = 0;
    }

    // channel swap as required from ABGR to ARGB
    uint32_t* t = (uint32_t*)pix;
    for (int i = 0; i < w * h; ++i) {
        uint32_t& rgb = t[i];
        rgb = ((rgb >> 16) & 0x000000ff) |
              ((rgb << 16) & 0x00ff0000) |
              ( rgb        & 0xff00ff00);
    }

    tengu::texture_t *tex = new tengu::texture_t;
    tex->w = w;
    tex->h = h;
    tex->channels = chans;
    tex->texel = pix;

    self->width = 1;
    self->height = 1;
    self->rendererObject = tex;
}

void _spAtlasPage_disposeTexture(spAtlasPage* self)
{
    tengu::texture_t* tex = (tengu::texture_t*)self->rendererObject;
    stbi_image_free(tex->texel);
    tex->texel = nullptr;
    delete tex;
    self->rendererObject = nullptr;
}

char* _spUtil_readFile(const char* path, int* length)
{
    tengu::buffer_t buf;
    if (!buf.load(path)) {
        return false;
    }
    *length = buf.size();
    return (char*)buf.release();
}

} // extern "C"

namespace tengu {

bool spine_prototype_t::load(const char* atlas_path, const char* json_path)
{

    _atlas = spAtlas_createFromFile(atlas_path, nullptr);
    if (!_atlas) {
        return false;
    }

    spSkeletonJson* json = spSkeletonJson_create(_atlas);
    if (!json) {
        return false;
    }
    json->scale = 1.f;
    _skel = spSkeletonJson_readSkeletonDataFile(json, json_path);
    spSkeletonJson_dispose(json);
    if (!_skel) {
        spAtlas_dispose(_atlas);
        return false;
    }

    return true;
}

bool spine_instance_t::load(spine_prototype_t& proto)
{
    _proto = &proto;

    _aState = spAnimationStateData_create(proto._skel);
    if (!_anim) {
        return false;
    }

    _aState->defaultMix = 0.f;

    _skel = spSkeleton_create(proto._skel);
    if (!_skel) {
        spAnimationStateData_dispose(_aState);
        return false;
    }

    _anim = spAnimationState_create(_aState);
    if (!_anim) {
        spSkeleton_dispose(_skel);
        spAnimationStateData_dispose(_aState);
        return false;
    }

    return true;
}

void spine_instance_t::tick(const float delta)
{
    spAnimationState_update(_anim, delta);
    spAnimationState_apply(_anim, _skel);
    spSkeleton_updateWorldTransform(_skel);
}

void spine_instance_t::set_animation(int track, const char* name, bool loop)
{
    spTrackEntry* t = spAnimationState_setAnimationByName(_anim, track, name, loop);
    //  t->mixTime = 0.1f;
}

void spine_instance_t::draw(draw_ex_t& draw)
{
    const texture_t *tex = (const texture_t*)_proto->_atlas->pages->rendererObject;

    // For each slot in the draw order array of the skeleton
    for (int i = 0; i < _skel->slotsCount; ++i) {
        spSlot* slot = _skel->drawOrder[i];

        // Fetch the currently active attachment, continue
        // with the next slot in the draw order if no
        // attachment is active on the slot
        spAttachment* attachment = slot->attachment;
        if (!attachment) {
            continue;
        }
        if (attachment->type != SP_ATTACHMENT_REGION) {
            continue;
        }

        // Fetch the blend mode from the slot and
        // translate it to the engine blend mode
        // slot->data->blendMode

        // Calculate the tinting color based on the skeleton's color
        // and the slot's color. Each color channel is given in the
        // range [0-1], you may have to multiply by 255 and cast to
        // and int if your engine uses integer ranges for color channels.
        //      float tintR = skeleton->r * slot->r;
        //      float tintG = skeleton->g * slot->g;
        //      float tintB = skeleton->b * slot->b;
        //      float tintA = skeleton->a * slot->a;

        // Fill the vertices array depending on the type of attachment
        int vertexIndex = 0;
        // Cast to an spRegionAttachment so we can get the rendererObject
        // and compute the world vertices
        spRegionAttachment* regionAttachment = (spRegionAttachment*)attachment;

        // Our engine specific Texture is stored in the spAtlasRegion which was
        // assigned to the attachment on load. It represents the texture atlas
        // page that contains the image the region attachment is mapped to
        //      texture = (Texture*)((spAtlasRegion*)regionAttachment->rendererObject)->page->rendererObject;

        // Computed the world vertices positions for the 4 vertices that make up
        // the rectangular region attachment. This assumes the world transform of the
        // bone to which the slot (and hence attachment) is attached has been calculated
        // before rendering via spSkeleton_updateWorldTransform
        float vert[16];
        spRegionAttachment_computeWorldVertices(regionAttachment, slot->bone, vert, 0, 2);

        draw.colour_ = 0xFFFFFF;
        for (int i = 0; i < 8; i += 2) {
            vert[i + 0] = 160 + vert[i + 0] / 3;
            vert[i + 1] = 240 - vert[i + 1] / 3;
        }

        draw.triangle(
            vec2f_t{ vert[0], vert[1] },
            vec2f_t{ vert[2], vert[3] },
            vec2f_t{ vert[4], vert[5] },
            vec2f_t{ regionAttachment->uvs[0], regionAttachment->uvs[1] },
            vec2f_t{ regionAttachment->uvs[2], regionAttachment->uvs[3] },
            vec2f_t{ regionAttachment->uvs[4], regionAttachment->uvs[5] },
            *tex);

        draw.triangle(
            vec2f_t{ vert[4], vert[5] },
            vec2f_t{ vert[6], vert[7] },
            vec2f_t{ vert[0], vert[1] },
            vec2f_t{ regionAttachment->uvs[4], regionAttachment->uvs[5] },
            vec2f_t{ regionAttachment->uvs[6], regionAttachment->uvs[7] },
            vec2f_t{ regionAttachment->uvs[0], regionAttachment->uvs[1] },
            *tex);
    }
}

} // namespace tengu
