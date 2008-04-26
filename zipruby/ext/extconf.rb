require 'mkmf'

if have_header('zlib.h') and have_library('z')
  create_makefile('zipruby')
end
