#include <ctype.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/hash.h>
#include <mruby/string.h>
#include <mruby/value.h>
#include <mruby/variable.h>
#include <mruby/presym.h>

#include "terminal_color.h"

#define mrb_terminal_color() mrb_module_get_id(mrb, MRB_SYM(TerminalColor))
#define mrb_termianl_color_const_get(sym) mrb_const_get(mrb, mrb_obj_value(mrb_terminal_color()), sym)

mrb_value color_code_4bit(mrb_state *mrb, mrb_value color, mrb_bool bg_color)
{
    if (mrb_nil_p(color) || !(mrb_symbol_p(color) || mrb_string_p(color)))
    {
        return mrb_nil_value();
    }

    mrb_value color_name = mrb_symbol_p(color) ? color : mrb_symbol_value(mrb_intern_str(mrb, color));

    mrb_sym table_name = bg_color ? MRB_SYM(ANSI_BG_COLOR_CODES) : MRB_SYM(ANSI_COLOR_CODES);
    mrb_value table = mrb_termianl_color_const_get(table_name);
    return mrb_hash_get(mrb, table, color_name);
}

mrb_value color_code_8bit(mrb_state *mrb, mrb_value color, mrb_bool badcheck)
{
    if (mrb_nil_p(color) || !(mrb_integer_p(color) || mrb_string_p(color)))
    {
        return mrb_nil_value();
    }

    mrb_int c = 0;

    if (mrb_integer_p(color))
    {
        c = mrb_integer(color);
    }
    else
    {
        mrb_int len = RSTRING_LEN(color);
        if (len == 0 || len > 3)
        {
            return mrb_nil_value();
        }
        const char *s = RSTRING_PTR(color);
        for (mrb_int i; i < len; i++)
        {
            if (!isdigit(s[i]))
            {
                return mrb_nil_value();
            }
            c = c * 10 + (s[i] - '0');
        }
    }
    if (c < 0 || c > 255)
    {
        if (badcheck)
        {
            return mrb_nil_value();
        }
        else
        {
            mrb_raisef(mrb, E_ARGUMENT_ERROR, "8bit color code out of range: %v", color);
        }
    }

    return mrb_int_value(mrb, c);
}

mrb_value color_code_24bit(mrb_state *mrb, mrb_value color, mrb_bool fallback)
{
    if (mrb_nil_p(color) || !(mrb_string_p(color) && RSTRING_PTR(color)[0] == '#'))
    {
        return mrb_nil_value();
    }

    if (RSTRING_LEN(color) != 7)
    {
        if (fallback)
        {
            return mrb_nil_value();
        }
        else
        {
            mrb_raisef(mrb, E_ARGUMENT_ERROR, "invalid hex code: %v", color);
        }
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

mrb_value mrb_value_to_color_code(mrb_state *mrb, mrb_value color, mrb_bool bg_color, mrb_bool fallback)
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

    color_code = color_code_24bit(mrb, color, fallback);
    if (!mrb_nil_p(color_code))
    {
        mrb_value color_code_str = mrb_str_new_cstr(mrb, bg_color ? "48;2;" : "38;2;");
        mrb_str_concat(mrb, color_code_str, color_code);
        return color_code_str;
    }

    color_code = color_code_8bit(mrb, color, fallback);
    if (!mrb_nil_p(color_code))
    {
        mrb_value color_code_str = mrb_str_new_cstr(mrb, bg_color ? "48;5;" : "38;5;");
        mrb_str_concat(mrb, color_code_str, color_code);
        return color_code_str;
    }

    if (!fallback)
    {
        mrb_raisef(mrb, E_ARGUMENT_ERROR, "unknown color: %v", color);
    }

    return mrb_nil_value();
}

mrb_value mrb_value_to_mode_code(mrb_state *mrb, mrb_value mode, mrb_bool fallback)
{
    if (mrb_nil_p(mode))
    {
        return mrb_nil_value();
    }

    if (mrb_symbol_p(mode) || mrb_string_p(mode))
    {
        mrb_sym name = mrb_symbol_p(mode) ? mrb_symbol(mode) : mrb_intern_str(mrb, mode);
        mrb_value table = mrb_termianl_color_const_get(MRB_SYM(ANSI_MODE_CODES));
        mrb_value mode_code = mrb_hash_get(mrb, table, mrb_symbol_value(name));
        if (!mrb_nil_p(mode_code))
        {
            return mode_code;
        }
    }

    if(!fallback)
    {
        mrb_raisef(mrb, E_ARGUMENT_ERROR, "unknown mode: %v", mode);
    }

    return mrb_nil_value();
}

mrb_bool has_escape_code(const char *str, mrb_int len)
{
    if (len < 2)
    {
        return FALSE;
    }
    return str[0] == '\e' && str[1] == '[';
}

mrb_bool has_reset_code(const char *str, mrb_int len)
{
    if (len < 3)
    {
        return FALSE;
    }
    return str[len - 3] == '\e' && str[len - 2] == '[' && str[len - 1] == 'm';
}

mrb_value build_escape_sequence(mrb_state *mrb, mrb_value fg_code, mrb_value bg_code, mrb_value mode_code)
{
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

    return result;
}

mrb_int count_newline(mrb_state *mrb, mrb_value str)
{
    mrb_int count = 0;
    const char *s = RSTRING_PTR(str);

    for (mrb_int i = 0; i < RSTRING_LEN(str); i++)
    {
        if (s[i] == '\n' || s[i] == '\r')
        {
            count++;
        }
    }

    return count;
}

mrb_int next_newline_pos(const char *s, size_t len)
{
    mrb_int result;

    for (result = 0; result < len; result++)
    {
        if (s[result] == '\n' || s[result] == '\r')
        {
            break;
        }
    }

    return result;
}

mrb_value mrb_str_set_color(mrb_state *mrb, mrb_value str, mrb_value color, mrb_value bg_color, mrb_value mode)
{
    if (RSTRING_LEN(str) == 0 || (mrb_nil_p(color) && mrb_nil_p(bg_color) && mrb_nil_p(mode)))
    {
        return str;
    }

    // only newline characters in the string
    mrb_int newline_count = count_newline(mrb, str);
    if (RSTRING_LEN(str) == newline_count)
    {
        return str;
    }

    mrb_value fg_code = mrb_value_to_color_code(mrb, color, FALSE, FALSE);
    mrb_value bg_code = mrb_value_to_color_code(mrb, bg_color, TRUE, FALSE);
    mrb_value mode_code = mrb_value_to_mode_code(mrb, mode, FALSE);

    mrb_value escape_sequence = build_escape_sequence(mrb, fg_code, bg_code, mode_code);
    const char *reset_code = "\e[m";
    mrb_value result = mrb_str_new_capa(mrb, RSTRING_LEN(str) + (RSTRING_LEN(escape_sequence) + 3) * newline_count);

    mrb_int len = RSTRING_LEN(str);
    const char *s = RSTRING_PTR(str);
    for (mrb_int i = 0; i < len; i++)
    {
        if (s[i] == '\n' || s[i] == '\r')
        {
            mrb_str_cat(mrb, result, &s[i], 1);
            continue;
        }

        mrb_str_concat(mrb, result, escape_sequence);

        mrb_int width = next_newline_pos(&s[i], len - i);
        if (has_escape_code(&s[i], width))
        {
            mrb_str_cat(mrb, result, ";", 1);
            mrb_str_cat(mrb, result, &s[i + 2], width - 2);
        }
        else
        {
            mrb_str_cat(mrb, result, "m", 1);
            mrb_str_cat(mrb, result, &s[i], width);
        }

        if (!has_reset_code(&s[i], width))
        {
            mrb_str_cat(mrb, result, reset_code, 3);
        }

        // decrement by 1 because the loop will increment it
        i += width - 1;
    }

    return result;
}

mrb_bool mrb_validate_color_code(mrb_state *mrb, mrb_value code)
{
    return mrb_test(code) && mrb_test(mrb_value_to_color_code(mrb, code, FALSE, TRUE));
}

mrb_bool mrb_validate_color_code_cstr(mrb_state *mrb, const char *str)
{
    return mrb_test(mrb_value_to_color_code(mrb, mrb_str_new_cstr(mrb, str), FALSE, TRUE));
}

mrb_bool mrb_validate_mode_code(mrb_state *mrb, mrb_value code)
{
    return mrb_test(code) && mrb_test(mrb_value_to_mode_code(mrb, code, TRUE));
}

mrb_bool mrb_validate_mode_code_cstr(mrb_state *mrb, const char *code)
{
    return mrb_test(mrb_value_to_mode_code(mrb, mrb_str_new_cstr(mrb, code), TRUE));
}

mrb_value mrb_terminal_color_set_color(mrb_state *mrb, mrb_value self)
{
    mrb_value color = mrb_nil_value();
    mrb_value bg_color = mrb_nil_value();
    mrb_value mode = mrb_nil_value();
    mrb_get_args(mrb, "o|oo", &color, &bg_color, &mode);

    return mrb_str_set_color(mrb, self, color, bg_color, mode);
}

mrb_value mrb_terminal_color_validate_color(mrb_state *mrb, mrb_value self)
{
    mrb_value color = mrb_nil_value();
    mrb_get_args(mrb, "o", &color);

    return mrb_bool_value(mrb_validate_color_code(mrb, color));
}

mrb_value mrb_terminal_color_validate_mode(mrb_state *mrb, mrb_value self)
{
    mrb_value mode = mrb_nil_value();
    mrb_get_args(mrb, "o", &mode);

    return mrb_bool_value(mrb_validate_mode_code(mrb, mode));
}

void mrb_mruby_terminal_color_gem_init(mrb_state *mrb)
{
    mrb_define_method_id(mrb, mrb->string_class, MRB_SYM(set_color), mrb_terminal_color_set_color, MRB_ARGS_REQ(1) | MRB_ARGS_OPT(2));

    struct RClass *t = mrb_define_module(mrb, "TerminalColor");
    mrb_define_module_function_id(mrb, t, mrb_intern_lit(mrb, "valid?"), mrb_terminal_color_validate_color, MRB_ARGS_REQ(1));
    mrb_define_module_function_id(mrb, t, mrb_intern_lit(mrb, "valid_mode?"), mrb_terminal_color_validate_mode, MRB_ARGS_REQ(1));
}

void mrb_mruby_terminal_color_gem_final(mrb_state *mrb)
{
}
