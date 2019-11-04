#include <set>

#include "framework_gui.h"


namespace tengu {

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

// modify hierarchy
void gui_frame_t::child_add(gui_frame_t* c) {
  child.push_back(c);
}

// remove a child
void gui_frame_t::child_remove(gui_frame_t* c) {
  for (auto itt = child.begin(); itt != child.end();) {
    if (*itt == c) {
      itt = child.erase(itt);
    }
    else {
      ++itt;
    }
  }
}

bool gui_frame_t::event_pop(gui_event_t &out, bool recurse) {
  if (event.empty()) {
    if (recurse) {
      for (gui_frame_t *f : child) {
        if (f->event_pop(out, true)) {
          return true;
        }
      }
    }
    return false;
  }
  else {
    out = event.back();
    event.pop_back();
    return true;
  }
}

void gui_frame_t::on_focus_lost() {
  gui_event_t e = {GUI_EVENT_FOCUS_LOST};
  event.push_back(e);
}

void gui_frame_t::on_focus_gain() {
  gui_event_t e = {GUI_EVENT_FOCUS_GAIN};
  event.push_back(e);
}

// auto size to fit children
void gui_frame_t::auto_size() {
  // TODO
}

// auto pack children
void gui_frame_t::auto_pack() {
  // TODO
}

void gui_frame_t::on_hover(gui_t &gui, gui_extern_io_t &io) {
  gui_extern_io_t::mouse_state_t mouse;
  io.mouse_get(mouse);
  // if we got clicked on then shift focus
  if (mouse.button[0] == gui_extern_io_t::IO_MOUSE_CLICK) {
    gui.set_focus(this);
  }
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

void gui_t::tick(gui_frame_t *frame,
                 gui_extern_io_t &io,
                 gui_extern_render_t &draw,
                 vec2i_t &origin,
                 gui_frame_t *&hover) {

  // update this frame
  frame->tick(*this, io, draw, origin);
  vec2i_t o = vec2i_t{origin.x + frame->x0, origin.y + frame->y0};

  // update all children
  for (gui_frame_t *c : frame->children()) {
    tick(c, io, draw, o, hover);
  }

  // check what mouse is hovering over
  if (!hover) {
    gui_extern_io_t::mouse_state_t mouse;
    io.mouse_get(mouse);
    if (mouse.x >= (origin.x + frame->x0) && mouse.x < (origin.x + frame->x1)) {
      if ((mouse.y >= origin.y + frame->y0) && mouse.y < (origin.y + frame->y1)) {
        hover = frame;
        frame->on_hover(*this, io);
      }
    }
  }

  if (focus) {
    // send keyboard events ?
    // send mouse move ?
  }
}

void gui_t::tick(gui_extern_io_t &io, gui_extern_render_t &draw) {
  vec2i_t origin = {0, 0};
  alloc.collect(*this);
  gui_frame_t *hover = nullptr;
  if (root) {
    tick(root, io, draw, origin, hover);
  }
}

bool gui_t::event_pop(gui_event_t &out) {
  return root->event_pop(out, true);
}

void gui_t::set_focus(gui_frame_t* f) {
  if (f != focus) {
    if (focus) {
      focus->on_focus_lost();
    }
    focus = f;
    focus->on_focus_gain();
  }
}

bool gui_t::has_focus(gui_frame_t* f) const {
  return f == focus;
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

void gui_alloc_t::collect(gui_t &gui) {
  if (gui.root) {
    std::set<gui_frame_t *> found;
    if (gui.focus) {
      found.insert(gui.focus);
    }
    found.insert(gui.root);
    visit(gui.root, found);
    for (auto itt = allocs.begin(); itt != allocs.end();) {
      if (found.count(*itt)) {
        ++itt;
      } else {
        delete *itt;
        itt = allocs.erase(itt);
      }
    }
  }
}

void gui_alloc_t::visit(gui_frame_t *f, std::set<gui_frame_t*> &found) const {
  for (gui_frame_t *c : f->children()) {
    found.insert(c);
    visit(c, found);
  }
}

}// namespace tengu
