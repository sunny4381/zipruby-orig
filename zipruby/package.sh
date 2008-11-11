#!/bin/sh
VERSION=0.3.0

rm *.gem *.tar.bz2
rm -rf doc
echo "/* rdoc source */" > zipruby.c
cat ext/*.c >> zipruby.c
cp ../libzip/*.* ext
rdoc README.txt zipruby.c LICENSE.libzip --title 'Zip/Ruby - Ruby bindings for libzip.'
tar jcvf zipruby-${VERSION}.tar.bz2 --exclude=.svn README.txt *.gemspec ext doc
gem build zipruby.gemspec
gem build zipruby-mswin32.gemspec
cp zipruby-${VERSION}-x86-mswin32.gem zipruby-${VERSION}-mswin32.gem
rm zipruby.c
for i in `ls ../libzip/*.*`
do
  rm ext/`basename $i`
done
