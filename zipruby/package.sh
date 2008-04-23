#!/bin/sh
rm *.gem *.tar.bz2
rm -rf doc
echo "/* rdoc source */" > zipruby.c
cat ext/*.c >> zipruby.c
rdoc zipruby.c --title 'Zip/Ruby - Ruby bindings for libzip.'
tar jcvf zipruby-0.1.0.tar.bz2 --exclude=.svn *.gemspec ext doc
gem build zipruby.gemspec
gem build zipruby-mswin32.gemspec
rm zipruby.c
