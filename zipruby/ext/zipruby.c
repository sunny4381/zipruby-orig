#ifdef _WIN32
__declspec(dllexport) void Init_zipruby(void);
#endif

#include "pkware_crc32.h"
#include "zipruby.h"
#include "zipruby_zip.h"
#include "zipruby_archive.h"
#include "zipruby_file.h"
#include "zipruby_stat.h"
#include "zipruby_error.h"

void Init_zipruby() {
  pkware_crc32_init_table();
  Init_zipruby_zip();
  Init_zipruby_archive();
  Init_zipruby_file();
  Init_zipruby_stat();
  Init_zipruby_error();
}
