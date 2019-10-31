#pragma once
#include <spine/spine.h>

#include "../framework_draw/draw.h"

namespace tengu {

struct spine_prototype_t {

  bool load(const char *atlas_path, const char *json_path);

  spAtlas *_atlas;
  spSkeletonData *_skel;
};

struct spine_instance_t {

  bool load(spine_prototype_t &proto);

  void draw(draw_ex_t &draw);
  void tick(const float delta);

  void set_animation(int track, const char *name, bool loop);

  spine_prototype_t *_proto;
  spAnimationStateData *_aState;
  spSkeleton *_skel;
  spAnimationState *_anim;
};

} // namespace tengu
