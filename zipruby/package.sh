#!/bin/sh
rm *.gem *.tar.bz2
rm -rf doc
echo "/* rdoc source */" > zipruby.c
cat ext/*.c >> zipruby.c
cp ../libzip/*.* ext
rdoc README.txt zipruby.c --title 'Zip/Ruby - Ruby bindings for libzip.'
tar jcvf zipruby-0.2.0.tar.bz2 --exclude=.svn README.txt *.gemspec ext doc
gem build zipruby.gemspec
gem build zipruby-mswin32.gemspec
rm zipruby.c
for i in `ls ../libzip/*.*`
do
  rm ext/`basename $i`
done
