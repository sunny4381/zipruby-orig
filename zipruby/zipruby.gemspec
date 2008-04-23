i386-mswin32Gem::Specification.new do |spec|
  spec.name              = 'zipruby'
  spec.version           = '0.1.0'
  spec.summary           = 'Ruby bindings for libzip.'
  spec.files             = %w(ext/zipruby.c zipruby.h ext/zipruby_archive.c ext/zipruby_archive.h ext/zipruby_file.c ext/zipruby_file.h ext/zipruby_stat.c ext/zipruby_stat.h ext/zipruby_zip.c ext/zipruby_zip.h ext/extconf.rb)
  spec.author            = 'winebarrel'
  spec.email             = 'sgwr_dts@yahoo.co.jp'
  spec.homepage          = 'http://zipruby.rubyforge.org'
  spec.extensions        = 'ext/extconf.rb'
  spec.has_rdoc          = true
  spec.rdoc_options      << '--title' << 'Zip/Ruby - Ruby bindings for libzip.'
  spec.extra_rdoc_files  = %w(ext/zipruby.c ext/zipruby_archive.c ext/zipruby_file.c ext/zipruby_stat.c ext/zipruby_zip.c)
  spec.rubyforge_project = 'zipruby'
end
