#ifndef __ZIPRUBY_STAT_H__
#define __ZIPRUBY_STAT_H__

#include "zip.h"

struct zipruby_stat {
  struct zip_stat *sb;
};

void Init_zipruby_stat();

#endif
