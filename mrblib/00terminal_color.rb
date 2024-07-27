module TerminalColor
  ANSI_COLOR_CODES = {
    default: 39,

    black: 30,
    red: 31,
    green: 32,
    yellow: 33,
    blue: 34,
    magenta: 35,
    cyan: 36,
    white: 37,
    gray: 90,
    bright_black: 90,
    bright_red: 91,
    bright_green: 92,
    bright_yellow: 93,
    bright_blue: 94,
    bright_magenta: 95,
    bright_cyan: 96,
    bright_white: 97
  }.freeze

  ANSI_BG_COLOR_CODES = {
    default: 49,

    black: 40,
    red: 41,
    green: 42,
    yellow: 43,
    blue: 44,
    magenta: 45,
    cyan: 46,
    white: 47,
    gray: 100,
    bright_black: 100,
    bright_red: 101,
    bright_green: 102,
    bright_yellow: 103,
    bright_blue: 104,
    bright_magenta: 105,
    bright_cyan: 106,
    bright_white: 107
  }.freeze

  ANSI_MODE_CODES = {
    reset: 0,
    bold: 1,
    underline: 4,
    blink: 5,
    reverse: 7,
    invisible: 8
  }.freeze
end
