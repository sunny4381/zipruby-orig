#include <string.h>

#include "zip.h"
#include "zipint.h"
#include "zipruby_zip_source_proc.h"
#include "ruby.h"

static ssize_t read_proc(void *state, void *data, size_t len, enum zip_source_cmd cmd) {
  struct read_proc *z;
  VALUE src;
  char *buf;
  size_t n;

  z = (struct read_proc *) state;
  buf = (char *) data;

  switch (cmd) {
  case ZIP_SOURCE_OPEN:
    return 0;

  case ZIP_SOURCE_READ:
    src = rb_funcall(z->proc, rb_intern("call"), 1, ULONG2NUM(len));

    if (NIL_P(src)) {
      return 0;
    }

    src = rb_check_convert_type(src, T_STRING, "String", "to_s");
    n = RSTRING(src)->len;

    if (n > 0) {
      n = (n > len) ? len : n;
      memcpy(buf, StringValuePtr(src), n);
    }

    return n;

  case ZIP_SOURCE_CLOSE:
    return 0;

  case ZIP_SOURCE_STAT:
    {
      struct zip_stat *st = (struct zip_stat *)data;
      zip_stat_init(st);
      // XXX: st->mtime = z->mtime
      st->mtime = time(NULL);
      return sizeof(*st);
    }

  case ZIP_SOURCE_ERROR:
    return 0;

  case ZIP_SOURCE_FREE:
    free(z);
    return 0;
  }

  return -1;
}

struct zip_source *zip_source_proc(struct zip *za, struct read_proc *z) {
  struct zip_source *zs;
  zs = zip_source_function(za, read_proc, z);
  return zs;
}
