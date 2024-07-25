#include <mruby.h>
#include <mruby/hash.h>
#include <mruby/string.h>
#include <mruby/value.h>
#include <mruby/variable.h>
#include <mruby/presym.h>

#include "mrb_terminal_color.h"

mrb_value color_code_4bit(mrb_state *mrb, mrb_value color, mrb_bool bg_color)
{
    if (mrb_nil_p(color) || !(mrb_symbol_p(color) || mrb_string_p(color)))
    {
        return mrb_nil_value();
    }

    mrb_value color_name = mrb_symbol_p(color) ? color : mrb_symbol_value(mrb_intern_str(mrb, color));

    mrb_sym table_name = bg_color ? MRB_SYM(ANSI_BG_COLOR_CODES) : MRB_SYM(ANSI_COLOR_CODES);
    mrb_value table = mrb_const_get(mrb, mrb_obj_value(mrb->string_class), table_name);
    return mrb_hash_get(mrb, table, color_name);
}

mrb_value color_code_8bit(mrb_state *mrb, mrb_value color)
{
    if (mrb_nil_p(color) || !(mrb_integer_p(color) || mrb_string_p(color)))
    {
        return mrb_nil_value();
    }

    mrb_value color_code = mrb_integer_p(color) ? color : mrb_str_to_integer(mrb, color, 10, TRUE);
    mrb_int c = mrb_integer(color_code);
    if (c < 0 || c > 255)
    {
        mrb_raisef(mrb, E_ARGUMENT_ERROR, "8bit color code out of range: %v", color);
        return mrb_nil_value();
    }
    return color_code;
}

mrb_value color_code_24bit(mrb_state *mrb, mrb_value color)
{
    if (mrb_nil_p(color) || !(mrb_string_p(color) && RSTRING_PTR(color)[0] == '#'))
    {
        return mrb_nil_value();
    }

    if (RSTRING_LEN(color) != 7)
    {
        mrb_raisef(mrb, E_ARGUMENT_ERROR, "invalid hex code: %v", color);
    }

    mrb_value color_hex = mrb_str_to_integer(mrb, mrb_str_new_cstr(mrb, RSTRING_PTR(color) + 1), 16, TRUE);

    mrb_value color_code = mrb_str_new_capa(mrb, 3 * 3 + 2); // RRR;GGG;BBB
    mrb_str_concat(mrb, color_code, mrb_int_value(mrb, mrb_integer(color_hex) >> 16 & 0xFF));
    mrb_str_concat(mrb, color_code, mrb_str_new_cstr(mrb, ";"));
    mrb_str_concat(mrb, color_code, mrb_int_value(mrb, mrb_integer(color_hex) >> 8 & 0xFF));
    mrb_str_concat(mrb, color_code, mrb_str_new_cstr(mrb, ";"));
    mrb_str_concat(mrb, color_code, mrb_int_value(mrb, mrb_integer(color_hex) & 0xFF));
    return color_code;
}

mrb_value mrb_value_to_color_code(mrb_state *mrb, mrb_value color, mrb_bool bg_color)
{
    if (mrb_nil_p(color))
    {
        return mrb_nil_value();
    }

    mrb_value color_code;
    color_code = color_code_4bit(mrb, color, bg_color);
    if (!mrb_nil_p(color_code))
    {
        return color_code;
    }

    color_code = color_code_24bit(mrb, color);
    if (!mrb_nil_p(color_code))
    {
        mrb_value color_code_str = mrb_str_new_cstr(mrb, bg_color ? "48;2;" : "38;2;");
        mrb_str_concat(mrb, color_code_str, color_code);
        return color_code_str;
    }

    color_code = color_code_8bit(mrb, color);
    if (!mrb_nil_p(color_code))
    {
        mrb_value color_code_str = mrb_str_new_cstr(mrb, bg_color ? "48;5;" : "38;5;");
        mrb_str_concat(mrb, color_code_str, color_code);
        return color_code_str;
    }

    mrb_raisef(mrb, E_ARGUMENT_ERROR, "unknown color: %v", color);
    return mrb_nil_value();
}

mrb_value mrb_value_to_mode_code(mrb_state *mrb, mrb_value mode)
{
    mrb_value table = mrb_const_get(mrb, mrb_obj_value(mrb->string_class), MRB_SYM(ANSI_MODE_CODES));
    return mrb_hash_get(mrb, table, mode);
}

mrb_bool mrb_str_has_escape_code(mrb_value str)
{
    const char *s = RSTRING_PTR(str);
    size_t len = RSTRING_LEN(str);
    if (len < 2)
    {
        return FALSE;
    }
    return s[0] == '\e' && s[1] == '[';
}

mrb_bool mrb_str_has_reset_code(mrb_value str)
{
    const char *s = RSTRING_PTR(str);
    size_t len = RSTRING_LEN(str);
    if (len < 3)
    {
        return FALSE;
    }
    return s[len - 3] == '\e' && s[len - 2] == '[' && s[len - 1] == 'm';
}

mrb_value mrb_str_set_color(mrb_state *mrb, mrb_value str, mrb_value color, mrb_value bg_color, mrb_value mode)
{
    mrb_value fg_code = mrb_value_to_color_code(mrb, color, FALSE);
    mrb_value bg_code = mrb_value_to_color_code(mrb, bg_color, TRUE);
    mrb_value mode_code = mrb_value_to_mode_code(mrb, mode);

    if (mrb_nil_p(fg_code) && mrb_nil_p(bg_code) && mrb_nil_p(mode_code))
    {
        return str;
    }

    mrb_value result = mrb_str_new_cstr(mrb, "\e[");
    if (!mrb_nil_p(fg_code))
    {
        mrb_str_concat(mrb, result, fg_code);
    }
    if (!mrb_nil_p(bg_code))
    {
        if (!mrb_nil_p(fg_code))
        {
            mrb_str_concat(mrb, result, mrb_str_new_cstr(mrb, ";"));
        }
        mrb_str_concat(mrb, result, bg_code);
    }
    if (!mrb_nil_p(mode_code))
    {
        if (!mrb_nil_p(fg_code) || !mrb_nil_p(bg_code))
        {
            mrb_str_concat(mrb, result, mrb_str_new_cstr(mrb, ";"));
        }
        mrb_str_concat(mrb, result, mode_code);
    }

    if (mrb_str_has_escape_code(str))
    {
        mrb_str_concat(mrb, result, mrb_str_new_cstr(mrb, ";"));
        size_t len = RSTRING_LEN(str);
        mrb_str_concat(mrb, result, mrb_str_substr(mrb, str, 2, len - 2));
    }
    else
    {
        mrb_str_concat(mrb, result, mrb_str_new_cstr(mrb, "m"));
        mrb_str_concat(mrb, result, str);
    }

    if (!mrb_str_has_reset_code(str))
    {
        mrb_str_concat(mrb, result, mrb_str_new_cstr(mrb, "\e[m"));
    }

    return result;
}

mrb_value mrb_terminal_color_set_color(mrb_state *mrb, mrb_value self)
{
    mrb_value color = mrb_nil_value();
    mrb_value bg_color = mrb_nil_value();
    mrb_value mode = mrb_nil_value();
    mrb_get_args(mrb, "o|oo", &color, &bg_color, &mode);

    return mrb_str_set_color(mrb, self, color, bg_color, mode);
}

void mrb_mruby_terminal_color_gem_init(mrb_state *mrb)
{
    mrb_define_method_id(mrb, mrb->string_class, MRB_SYM(set_color), mrb_terminal_color_set_color, MRB_ARGS_REQ(1) | MRB_ARGS_OPT(2));
}

void mrb_mruby_terminal_color_gem_final(mrb_state *mrb)
{
}
