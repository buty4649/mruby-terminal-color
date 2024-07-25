MRuby::Build.new do |conf|
  conf.toolchain

  conf.gembox 'default'
  conf.gem File.expand_path(__dir__)

  conf.enable_debug
  conf.enable_test
end
