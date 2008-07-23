#ifndef __ZIPRUBY_H__
#define __ZIPRUBY_H__

#include "ruby.h"

#ifndef RSTRING_PTR
#define RSTRING_PTR(s) (RSTRING(s)->ptr)
#endif
#ifndef RSTRING_LEN
#define RSTRING_LEN(s) (RSTRING(s)->len)
#endif

#define VERSION "0.2.9"
#define ERRSTR_BUFSIZE 256
#define DATA_BUFSIZE 8192

void Init_zipruby();

#endif
