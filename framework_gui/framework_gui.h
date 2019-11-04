#pragma once
#include <stdint.h>

#include <vector>
#include <set>

#include "../framework_core/common.h"
#include "../framework_core/vec2.h"


namespace tengu {

struct gui_frame_t;
struct gui_t;
struct gui_extern_render_t;
struct gui_extern_io_t;

enum gui_event_type_t {
  GUI_EVENT_FOCUS_LOST,
  GUI_EVENT_FOCUS_GAIN,
  GUI_EVENT_BUTTON_PUSHED,
  GUI_EVENT_CHECKED,
  GUI_EVENT_UNCHECKED,
  GUI_EVENT_SLIDER,
};

struct gui_event_t {
  int32_t type;
};

struct gui_frame_t {

  // event list
  std::vector<gui_event_t> event;

  // bounds
  // TODO: make a rect_t?
  int32_t x0, y0, x1, y1;

  // frame type
  uint32_t type;

  // custom widget tag
  uint32_t user_tag;

  template <typename TYPE>
  bool is_a() const {
    return type == TYPE::_type;
  }

  template <typename TYPE>
  TYPE *cast() {
    return is_a<TYPE>() ? (TYPE*)this : nullptr;
  }

  // external interaction
  virtual void tick(
    gui_t &gui,
    gui_extern_io_t &io,
    gui_extern_render_t &draw,
    vec2i_t &origin) = 0;

  // modify hierarchy
  virtual void child_add(gui_frame_t*);
  virtual void child_remove(gui_frame_t*);

  // focus change
  virtual void on_focus_lost();
  virtual void on_focus_gain();

  //
  virtual void on_hover(gui_t &gui, gui_extern_io_t &io);

  const auto &children() const {
    return child;
  }

  bool event_pop(gui_event_t &out, bool recurse);

  // auto size to fit children
  void auto_size();

  // auto pack children
  void auto_pack();

protected:
  gui_frame_t(int32_t t)
    : type(t)
    , user_tag(0)
    , parent(nullptr)
  {}

  // hierarchy
  std::vector<gui_frame_t*> child;
  gui_frame_t *parent;
};

struct gui_extern_render_t {

  // the current draw color
  uint32_t rgb;

  // drawing origin
  vec2i_t origin;

  // draw outline rectangle
  virtual void draw_rect_outline( int32_t x0, int32_t y0, int32_t x1, int32_t y1) = 0;

  // draw filled rectangle
  virtual void draw_rect_fill( int32_t x0, int32_t y0, int32_t x1, int32_t y1) = 0;

  // draw transparent shadow
  virtual void draw_shadow( int32_t x0, int32_t y0, int32_t x1, int32_t y1) = 0;

  // draw rounded rectangle
  virtual void draw_circle(int32_t x0, int32_t y0, int32_t r) = 0;

  // draw text
  virtual void draw_text(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const std::string &txt) = 0;
};

struct gui_extern_io_t {

  enum {
    IO_MOUSE_RELEASE = -1,
    IO_MOUSE_UP      =  0,
    IO_MOUSE_DOWN    =  1,
    IO_MOUSE_CLICK   =  2,
  };

  struct mouse_state_t {
    int32_t x, y;
    int32_t button[3];
  };

  virtual void mouse_get(mouse_state_t &out) = 0;

  struct key_state_t {
    uint8_t key[256];
  };

  virtual void key_get(key_state_t &key) = 0;
};

struct gui_alloc_t {

  template <typename TYPE_T>
  TYPE_T *frame_create() {
    TYPE_T *f = new TYPE_T();
    allocs.push_back(f);
    return f;
  }

  void collect(gui_t &gui);

  std::vector<gui_frame_t*> allocs;

protected:
  void visit(gui_frame_t*, std::set<gui_frame_t*> &found) const;
};

struct gui_t {

  friend struct gui_alloc_t;

  gui_t()
    : root(nullptr)
    , focus(nullptr)
  {}

  // update the gui
  void tick(gui_extern_io_t &io, gui_extern_render_t &draw);

  // pop next event from available frame
  bool event_pop(gui_event_t &out);

  gui_alloc_t alloc;

  // root frame
  gui_frame_t* root;

  void set_focus(gui_frame_t* f);
  bool has_focus(gui_frame_t* f) const;

protected:
  void tick(gui_frame_t *frame,
            gui_extern_io_t &io,
            gui_extern_render_t &draw,
            vec2i_t &origin,
            gui_frame_t *&hover);

  gui_frame_t* focus;
};

} // namespace tengu
