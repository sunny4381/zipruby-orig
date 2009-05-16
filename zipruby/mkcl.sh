#!/bin/sh
REV=`cat .revision`
svn2cl -i -a --group-by-day --stdout -r HEAD:198
