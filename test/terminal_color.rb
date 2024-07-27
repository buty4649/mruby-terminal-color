assert('TerminalColor.valid?') do
  %i[
    default black red green yellow blue magenta cyan white gray
    bright_black bright_red bright_green bright_yellow bright_blue
    bright_magenta bright_cyan bright_white
  ].each do |code|
    assert_true TerminalColor.valid?(code), ":#{code}"
    assert_true TerminalColor.valid?(code.to_s), "\"#{code}\""
  end
  assert_false TerminalColor.valid?(:invalid), ':invalid'

  assert_true TerminalColor.valid?(0), '8bit color(0)'
  assert_true TerminalColor.valid?(255), '8bit color(255)'
  assert_false TerminalColor.valid?(-1), 'invalid 8bit color(-1)'
  assert_false TerminalColor.valid?(256), 'invalid 8bit color(256)'

  assert_true TerminalColor.valid?('#f0f0f0'), '24bit color(#f0f0f0)'
  assert_false TerminalColor.valid?('f0f0f0'), 'invalid 24bit color(f0f0f0)'
  assert_false TerminalColor.valid?('#fff'), 'invalid 24bit color(#fff)'

  assert_false TerminalColor.valid?(nil), 'nil'
  assert_false TerminalColor.valid?([]), 'Array'
end

assert('TerminalColor.valid_mode?') do
  %i[reset bold underline blink reverse invisible].each do |code|
     assert_true TerminalColor.valid_mode?(code), ":#{code}"
     assert_true TerminalColor.valid_mode?(code.to_s), "\"#{code}\""
  end
  assert_false TerminalColor.valid_mode?(:invalid), ':invalid'

  assert_false TerminalColor.valid_mode?(nil), 'nil'
  assert_false TerminalColor.valid_mode?([]), 'Array'
end
