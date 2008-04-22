require 'mkmf'

if have_header('zip.h') and have_library('zip') and have_library('z')
  create_makefile('zipruby')
end
