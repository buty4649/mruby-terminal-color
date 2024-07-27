class String
  alias color set_color

  TerminalColor::ANSI_COLOR_CODES.each_key do |color|
    next if color == :default

    define_method(color) do
      set_color(color.to_sym)
    end
  end

  TerminalColor::ANSI_BG_COLOR_CODES.each_key do |color|
    next if color == :default

    define_method("bg_#{color}") do
      set_color(nil, color.to_sym)
    end
  end

  TerminalColor::ANSI_MODE_CODES.each_key do |mode|
    next if mode == :reset

    define_method(mode) do
      set_color(nil, nil, mode.to_sym)
    end
  end
end
