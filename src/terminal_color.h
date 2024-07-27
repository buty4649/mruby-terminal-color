#ifndef _MRB_TERMINAL_COLOR_H_
#define _MRB_TERMINAL_COLOR_H_

#include <mruby.h>

mrb_value mrb_str_set_color(mrb_state *mrb, mrb_value str, mrb_value color, mrb_value bg_color, mrb_value mode);

#endif // _MRB_TERMINAL_COLOR_H_
