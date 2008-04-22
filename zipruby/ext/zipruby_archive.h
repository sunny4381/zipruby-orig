#ifndef __ZIPRUBY_ARCHIVE_H__
#define __ZIPRUBY_ARCHIVE_H__

#include "zip.h"

struct zipruby_archive {
  struct zip *archive;
};

void Init_zipruby_archive();

#define Check_Archive(p) do { \
  if ((p)->archive == NULL) { \
    rb_raise(rb_eRuntimeError, "invalid Zip::Archive"); \
  } \
} while(0)

#endif
