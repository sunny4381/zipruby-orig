#ifndef __ZIPRUBY_H__
#define __ZIPRUBY_H__

#include "ruby.h"

#ifndef RSTRING_PTR
#define RSTRING_PTR(s) (RSTRING(s)->ptr)
#endif
#ifndef RSTRING_LEN
#define RSTRING_LEN(s) (RSTRING(s)->len)
#endif

#define Check_IO(x) do { \
  if (!rb_obj_is_instance_of((x), rb_cIO)) { \
    rb_raise(rb_eTypeError, "wrong argument type %s (expected IO)", rb_class2name(CLASS_OF(x))); \
  } \
} while(0)

#define VERSION "0.2.9"
#define ERRSTR_BUFSIZE 256
#define DATA_BUFSIZE 8192

void Init_zipruby();

#endif
