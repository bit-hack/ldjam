#pragma once
#include "framework_gui.h"

namespace tengu {

enum gui_widget_type_t {
  GUI_WIDGET_FRAME,
  GUI_WIDGET_BUTTON,
  GUI_WIDGET_CHECK_BOX,
  GUI_WIDGET_RADIO_BUTTON,
  GUI_WIDGET_TEXT_BOX,
  GUI_WIDGET_TEXT,
  GUI_WIDGET_HSLIDER,
  GUI_WIDGET_VSLIDER,
  GUI_WIDGET_PROGRESS_BAR,
  GUI_WIDGET_COMBO_BOX,
  GUI_WIDGET_LIST_BOX,
};

struct gui_widget_frame_t: public gui_frame_t {

  gui_widget_frame_t()
    : gui_frame_t(_type) {
  }

  static const uint32_t _type = GUI_WIDGET_FRAME;

  void tick(gui_t &,
            gui_extern_io_t &io,
            gui_extern_render_t &draw,
            vec2i_t &origin) override;
};

struct gui_widget_button_t: public gui_frame_t {

  gui_widget_button_t()
    : gui_frame_t(_type)
  {
  }

  std::string caption;

  static const uint32_t _type = GUI_WIDGET_BUTTON;

  void tick(gui_t &,
            gui_extern_io_t &io,
            gui_extern_render_t &draw,
            vec2i_t &origin) override;
};

struct gui_widget_check_box_t: public gui_frame_t {

  static const uint32_t width  = 11;
  static const uint32_t height = 11;

  gui_widget_check_box_t()
    : gui_frame_t(_type)
    , checked(false) {
  }

  static const uint32_t _type = GUI_WIDGET_CHECK_BOX;

  bool checked;

  void tick(gui_t &,
            gui_extern_io_t &io,
            gui_extern_render_t &draw,
            vec2i_t &origin) override;
};

struct gui_widget_hslider_t: public gui_frame_t {

  static const uint32_t size = 9;

  gui_widget_hslider_t()
    : gui_frame_t(_type)
    , max_value(100)
    , value(0) {
  }

  static const uint32_t _type = GUI_WIDGET_HSLIDER;

  int32_t max_value;
  int32_t value;

  void tick(gui_t &,
            gui_extern_io_t &io,
            gui_extern_render_t &draw,
            vec2i_t &origin) override;
};

struct gui_widget_vslider_t: public gui_frame_t {

  static const uint32_t size = 9;

  gui_widget_vslider_t()
    : gui_frame_t(_type)
    , max_value(100)
    , value(0) {
  }

  static const uint32_t _type = GUI_WIDGET_VSLIDER;

  int32_t max_value;
  int32_t value;

  void tick(gui_t &,
            gui_extern_io_t &io,
            gui_extern_render_t &draw,
            vec2i_t &origin) override;
};

struct gui_widget_progress_bar_t: public gui_frame_t {

  static const uint32_t size = 9;

  gui_widget_progress_bar_t()
    : gui_frame_t(_type)
    , max_value(100)
    , value(0) {
  }

  static const uint32_t _type = GUI_WIDGET_PROGRESS_BAR;

  int32_t max_value;
  int32_t value;

  void tick(gui_t &,
            gui_extern_io_t &io,
            gui_extern_render_t &draw,
            vec2i_t &origin) override;
};

} // namespage tengu
