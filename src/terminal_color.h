#ifndef _MRB_TERMINAL_COLOR_H_
#define _MRB_TERMINAL_COLOR_H_

#include <mruby.h>

#ifdef __cplusplus
extern "C" {
#endif

mrb_value mrb_str_set_color(mrb_state *mrb, mrb_value str, mrb_value color, mrb_value bg_color, mrb_value mode);

mrb_bool mrb_validate_color_code(mrb_state *mrb, mrb_value code);
mrb_bool mrb_validate_color_code_cstr(mrb_state *mrb, const char *code);
mrb_bool mrb_validate_mode_code(mrb_state *mrb, mrb_value code);
mrb_bool mrb_validate_mode_code_cstr(mrb_state *mrb, const char* mode);

#ifdef __cplusplus
}
#endif

#endif // _MRB_TERMINAL_COLOR_H_
