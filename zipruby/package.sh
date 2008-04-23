#!/bin/sh
rm *.gem *.tar.bz2
rm -rf doc
rdoc ext/zipruby.c ext/zipruby_archive.c ext/zipruby_file.c ext/zipruby_stat.c ext/zipruby_zip.c --title 'Zip/Ruby - Ruby bindings for libzip.'
tar jcvf zipruby-0.1.0.tar.bz2 --exclude=.svn *.gemspec ext doc
gem build zipruby.gemspec
gem build zipruby-mswin32.gemspec
