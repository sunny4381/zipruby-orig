Gem::Specification.new do |spec|
  spec.name              = 'zipruby'
  spec.version           = '0.0.1'
  spec.platform          = 'mswin32'
  spec.summary           = 'Ruby bindings for libzip.'
  spec.require_paths     = %w(lib/i386-mswin32)
  spec.files             = %w(lib/i386-mswin32/zipruby.so)
  spec.author            = 'winebarrel'
  spec.email             = 'sgwr_dts@yahoo.co.jp'
  spec.homepage          = 'http://zipruby.rubyforge.org'
  spec.has_rdoc          = true
  spec.rdoc_options      << '--title' << 'Zip/Ruby - Ruby bindings for libzip.'
  spec.extra_rdoc_files  = %w(ext/zipruby.c ext/zipruby_archive.c ext/zipruby_file.c ext/zipruby_stat.c ext/zipruby_zip.c)
  spec.rubyforge_project = 'zipruby'
end
