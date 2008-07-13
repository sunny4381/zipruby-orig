#include <errno.h>

#include "zip.h"
#include "zipruby.h"
#include "zipruby_archive.h"
#include "zipruby_zip_source_proc.h"
#include "ruby.h"
#include "rubyio.h"

static VALUE zipruby_archive_alloc(VALUE klass);
static void zipruby_archive_free(struct zipruby_archive *p);
static VALUE zipruby_archive_s_open(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_s_decrypt(VALUE self, VALUE path, VALUE password);
static VALUE zipruby_archive_s_encrypt(VALUE self, VALUE path, VALUE password);
static VALUE zipruby_archive_close(VALUE self);
static VALUE zipruby_archive_num_files(VALUE self);
static VALUE zipruby_archive_get_name(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_fopen(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_get_stat(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_add_buffer(VALUE self, VALUE name, VALUE source);
static VALUE zipruby_archive_add_file(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_add_filep(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_add_function(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_replace_buffer(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_replace_file(int argc, VALUE* argv, VALUE self);
static VALUE zipruby_archive_replace_filep(VALUE self, VALUE index, VALUE file);
static VALUE zipruby_archive_replace_function(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_add_or_replace_buffer(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_add_or_replace_file(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_add_or_replace_filep(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_add_or_replace_function(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_update(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_get_comment(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_set_comment(VALUE self, VALUE comment);
static VALUE zipruby_archive_locate_name(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_get_fcomment(int argc, VALUE *argv, VALUE self);
static VALUE zipruby_archive_set_fcomment(VALUE self, VALUE index, VALUE comment);
static VALUE zipruby_archive_fdelete(VALUE self, VALUE index);
static VALUE zipruby_archive_frename(VALUE self, VALUE index, VALUE name);
static VALUE zipruby_archive_funchange(VALUE self, VALUE index);
static VALUE zipruby_archive_funchange_all(VALUE self);
static VALUE zipruby_archive_unchange(VALUE self);
static VALUE zipruby_archive_revert(VALUE self);
static VALUE zipruby_archive_each(VALUE self);

extern VALUE Zip;
VALUE Archive;
extern VALUE File;
extern VALUE Stat;
extern VALUE Error;

void Init_zipruby_archive() {
  Archive = rb_define_class_under(Zip, "Archive", rb_cObject);
  rb_define_alloc_func(Archive, zipruby_archive_alloc);
  rb_include_module(Archive, rb_mEnumerable);
  rb_define_singleton_method(Archive, "open", zipruby_archive_s_open, -1);
  rb_define_singleton_method(Archive, "decrypt", zipruby_archive_s_decrypt, 2);
  rb_define_singleton_method(Archive, "encrypt", zipruby_archive_s_encrypt, 2);
  rb_define_method(Archive, "close", zipruby_archive_close, 0);
  rb_define_method(Archive, "num_files", zipruby_archive_num_files, 0);
  rb_define_method(Archive, "get_name", zipruby_archive_get_name, -1);
  rb_define_method(Archive, "fopen", zipruby_archive_fopen, -1);
  rb_define_method(Archive, "get_stat", zipruby_archive_get_stat, -1);
  rb_define_method(Archive, "add_buffer", zipruby_archive_add_buffer, 2);
  rb_define_method(Archive, "add_file", zipruby_archive_add_file, -1);
  rb_define_method(Archive, "add_filep", zipruby_archive_add_filep, -1);
  rb_define_method(Archive, "add", zipruby_archive_add_function, -1);
  rb_define_method(Archive, "replace_buffer", zipruby_archive_replace_buffer, -1);
  rb_define_method(Archive, "replace_file", zipruby_archive_replace_file, -1);
  rb_define_method(Archive, "replace_filep", zipruby_archive_replace_filep, 2);
  rb_define_method(Archive, "replace", zipruby_archive_replace_function, -1);
  rb_define_method(Archive, "add_or_replace_buffer", zipruby_archive_add_or_replace_buffer, -1);
  rb_define_method(Archive, "add_or_replace_file", zipruby_archive_add_or_replace_file, -1);
  rb_define_method(Archive, "add_or_replace_filep", zipruby_archive_add_or_replace_filep, -1);
  rb_define_method(Archive, "add_or_replace", zipruby_archive_add_or_replace_function, -1);
  rb_define_method(Archive, "update", zipruby_archive_update, -1);
  rb_define_method(Archive, "<<", zipruby_archive_add_filep, -1);
  rb_define_method(Archive, "get_comment", zipruby_archive_get_comment, -1);
  rb_define_method(Archive, "comment", zipruby_archive_get_comment, -1);
  rb_define_method(Archive, "comment=", zipruby_archive_set_comment, 1);
  rb_define_method(Archive, "locate_name", zipruby_archive_locate_name, -1);
  rb_define_method(Archive, "get_fcomment", zipruby_archive_get_fcomment, -1);
  rb_define_method(Archive, "set_fcomment", zipruby_archive_set_fcomment, 2);
  rb_define_method(Archive, "fdelete", zipruby_archive_fdelete, 1);
  rb_define_method(Archive, "frename", zipruby_archive_frename, 2);
  rb_define_method(Archive, "funchange", zipruby_archive_funchange, 1);
  rb_define_method(Archive, "funchange_all", zipruby_archive_funchange_all, 0);
  rb_define_method(Archive, "unchange", zipruby_archive_unchange, 0);
  rb_define_method(Archive, "frevert", zipruby_archive_unchange, 1);
  rb_define_method(Archive, "revert", zipruby_archive_revert, 0);
  rb_define_method(Archive, "each", zipruby_archive_each, 0);
}

static VALUE zipruby_archive_alloc(VALUE klass) {
  struct zipruby_archive *p = ALLOC(struct zipruby_archive);

  p->archive = NULL;

  return Data_Wrap_Struct(klass, 0, zipruby_archive_free, p);
}

static void zipruby_archive_free(struct zipruby_archive *p) {
  xfree(p);
}

/* */
static VALUE zipruby_archive_s_open(int argc, VALUE *argv, VALUE self) {
  VALUE path, flags;
  VALUE archive;
  struct zipruby_archive *p_archive;
  int i_flags = 0;
  int errorp;

  rb_scan_args(argc, argv, "11", &path, &flags);
  Check_Type(path, T_STRING);

  if (!NIL_P(flags)) {
    i_flags = NUM2INT(flags);
  }

  archive = rb_funcall(Archive, rb_intern("new"), 0);
  Data_Get_Struct(archive, struct zipruby_archive, p_archive);

  if ((p_archive->archive = zip_open(StringValuePtr(path), i_flags, &errorp)) == NULL) {
    char errstr[ERRSTR_BUFSIZE];
    zip_error_to_str(errstr, ERRSTR_BUFSIZE, errorp, errno);
    rb_raise(Error, "Open archive failed - %s: %s", StringValuePtr(path), errstr);
  }

  if (rb_block_given_p()) {
    VALUE retval;
    int status;

    retval = rb_protect(rb_yield, archive, &status);
    zipruby_archive_close(archive);

    if (status != 0) {
      rb_jump_tag(status);
    }

    return retval;
  } else {
    return archive;
  }
}

/* */
static VALUE zipruby_archive_s_decrypt(VALUE self, VALUE path, VALUE password) {
  int errorp, wrongpwd;
  long pwdlen;

  Check_Type(path, T_STRING);
  Check_Type(password, T_STRING);
  pwdlen = RSTRING(password)->len;

  if (pwdlen < 1) {
    rb_raise(Error, "Decrypt archive failed - %s: Password is empty", StringValuePtr(path));
  } else if (pwdlen > 0xff) {
    rb_raise(Error, "Decrypt archive failed - %s: Password is too long", StringValuePtr(path));
  }

  if (zip_decrypt(StringValuePtr(path), StringValuePtr(password), pwdlen, &errorp, &wrongpwd) == -1) {
    if (wrongpwd) {
      rb_raise(Error, "Decrypt archive failed - %s: Wrong password", StringValuePtr(path));
    } else {
      char errstr[ERRSTR_BUFSIZE];
      zip_error_to_str(errstr, ERRSTR_BUFSIZE, errorp, errno);
      rb_raise(Error, "Decrypt archive failed - %s: %s", StringValuePtr(path), errstr);
    }
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_s_encrypt(VALUE self, VALUE path, VALUE password) {
  int errorp;
  long pwdlen;

  Check_Type(path, T_STRING);
  Check_Type(password, T_STRING);
  pwdlen = RSTRING(password)->len;

  if (pwdlen < 1) {
    rb_raise(Error, "Encrypt archive failed - %s: Password is empty", StringValuePtr(path));
  } else if (pwdlen > 0xff) {
    rb_raise(Error, "Encrypt archive failed - %s: Password is too long", StringValuePtr(path));
  }

  if (zip_encrypt(StringValuePtr(path), StringValuePtr(password), pwdlen, &errorp) == -1) {
    char errstr[ERRSTR_BUFSIZE];
    zip_error_to_str(errstr, ERRSTR_BUFSIZE, errorp, errno);
    rb_raise(Error, "Encrypt archive failed - %s: %s", StringValuePtr(path), errstr);
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_close(VALUE self) {
  struct zipruby_archive *p_archive;

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  if (zip_close(p_archive->archive) == -1) {
    zip_unchange_all(p_archive->archive);
    zip_unchange_archive(p_archive->archive);
    rb_raise(Error, "Close archive failed: %s", zip_strerror(p_archive->archive));
  }

  p_archive->archive = NULL;

  return Qnil;
}

/* */
static VALUE zipruby_archive_num_files(VALUE self) {
  struct zipruby_archive *p_archive;
  int num_files;

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);
  num_files = zip_get_num_files(p_archive->archive);

  return INT2NUM(num_files);
}

/* */
static VALUE zipruby_archive_get_name(int argc, VALUE *argv, VALUE self) {
  VALUE index, flags;
  struct zipruby_archive *p_archive;
  int i_flags = 0;
  const char *name;

  rb_scan_args(argc, argv, "11", &index, &flags);
  Check_Type(index, T_FIXNUM);

  if (!NIL_P(flags)) {
    i_flags = NUM2INT(flags);
  }

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  if ((name = zip_get_name(p_archive->archive, NUM2INT(index), i_flags)) == NULL) {
    rb_raise(Error, "Get name failed at %d: %s", index, zip_strerror(p_archive->archive));
  }

  return (name != NULL) ? rb_str_new2(name) : Qnil;
}

/* */
static VALUE zipruby_archive_fopen(int argc, VALUE *argv, VALUE self) {
  VALUE index, flags, stat_flags, file;

  rb_scan_args(argc, argv, "12", &index, &flags, &stat_flags);
  file = rb_funcall(File, rb_intern("new"), 4, self, index, flags, stat_flags);

  if (rb_block_given_p()) {
    VALUE retval;
    int status;

    retval = rb_protect(rb_yield, file, &status);
    rb_funcall(file, rb_intern("close"), 0);

    if (status != 0) {
      rb_jump_tag(status);
    }

    return retval;
  } else {
    return file;
  }
}

/* */
static VALUE zipruby_archive_get_stat(int argc, VALUE *argv, VALUE self) {
  VALUE index, flags;

  rb_scan_args(argc, argv, "11", &index, &flags);

  return rb_funcall(Stat, rb_intern("new"), 3, self, index, flags);
}

/* */
static VALUE zipruby_archive_add_buffer(VALUE self, VALUE name, VALUE source) {
  struct zipruby_archive *p_archive;
  struct zip_source *zsource;
  char *data;
  off_t len;

  Check_Type(name, T_STRING);
  Check_Type(source, T_STRING);
  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  len = RSTRING(source)->len;

  if ((data = malloc(len)) == NULL) {
    rb_raise(rb_eRuntimeError, "Add file failed: Cannot allocate memory");
  }

  memset(data, 0, len);
  memcpy(data, StringValuePtr(source), len);

  if ((zsource = zip_source_buffer(p_archive->archive, data, len, 1)) == NULL) {
    free(data);
    rb_raise(Error, "Add file failed - %s: %s", StringValuePtr(name), zip_strerror(p_archive->archive));
  }

  if (zip_add(p_archive->archive, StringValuePtr(name), zsource) == -1) {
    zip_source_free(zsource);
    zip_unchange_all(p_archive->archive);
    zip_unchange_archive(p_archive->archive);
    rb_raise(Error, "Add file failed - %s: %s", StringValuePtr(name), zip_strerror(p_archive->archive));
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_replace_buffer(int argc, VALUE *argv, VALUE self) {
  struct zipruby_archive *p_archive;
  struct zip_source *zsource;
  VALUE index, source, flags;
  int i_index, i_flags = 0;
  char *data;
  off_t len;

  rb_scan_args(argc, argv, "21", &index, &source, &flags);

  if (!rb_obj_is_instance_of(index, rb_cString) && !rb_obj_is_instance_of(index, rb_cFixnum)) {
    rb_raise(rb_eTypeError, "wrong argument type %s (expected Fixnum or String)", rb_class2name(CLASS_OF(index)));
  }

  if (!NIL_P(flags)) {
    i_flags = NUM2INT(flags);
  }

  Check_Type(source, T_STRING);
  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  if (rb_obj_is_instance_of(index, rb_cFixnum)) {
    i_index = NUM2INT(index);
  } else if ((i_index = zip_name_locate(p_archive->archive, StringValuePtr(index), i_flags)) == -1) {
    rb_raise(Error, "Replace file failed - %s: Archive does not contain a file", StringValuePtr(index));
  }

  len = RSTRING(source)->len;

  if ((data = malloc(len)) == NULL) {
    rb_raise(rb_eRuntimeError, "Replace file failed: Cannot allocate memory");
  }

  memcpy(data, StringValuePtr(source), len);

  if ((zsource = zip_source_buffer(p_archive->archive, data, len, 1)) == NULL) {
    free(data);
    rb_raise(Error, "Replace file failed at %d: %s", i_index, zip_strerror(p_archive->archive));
  }

  if (zip_replace(p_archive->archive, i_index, zsource) == -1) {
    zip_source_free(zsource);
    zip_unchange_all(p_archive->archive);
    zip_unchange_archive(p_archive->archive);
    rb_raise(Error, "Replace file failed at %d: %s", i_index, zip_strerror(p_archive->archive));
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_add_or_replace_buffer(int argc, VALUE *argv, VALUE self) {
  struct zipruby_archive *p_archive;
  VALUE name, source, flags;
  int index, i_flags = 0;

  rb_scan_args(argc, argv, "21", &name, &source, &flags);

  if (!NIL_P(flags)) {
    i_flags = NUM2INT(flags);
  }

  Check_Type(name, T_STRING);
  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  index = zip_name_locate(p_archive->archive, StringValuePtr(name), i_flags);

  if (index >= 0) {
    VALUE _args[] = { INT2NUM(index), source };
    return zipruby_archive_replace_buffer(2, _args, self);
  } else {
    return zipruby_archive_add_buffer(self, name, source);
  }
}

/* */
static VALUE zipruby_archive_add_file(int argc, VALUE *argv, VALUE self) {
  VALUE name, fname;
  struct zipruby_archive *p_archive;
  struct zip_source *zsource;

  rb_scan_args(argc, argv, "11", &name, &fname);

  if (NIL_P(fname)) {
    fname = name;
    name = Qnil;
  }

  Check_Type(fname, T_STRING);

  if (NIL_P(name)) {
    name = rb_funcall(rb_cFile, rb_intern("basename"), 1, fname);
  }

  Check_Type(name, T_STRING);
  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  if ((zsource = zip_source_file(p_archive->archive, StringValuePtr(fname), 0, -1)) == NULL) {
    rb_raise(Error, "Add file failed - %s: %s", StringValuePtr(name), zip_strerror(p_archive->archive));
  }

  if (zip_add(p_archive->archive, StringValuePtr(name), zsource) == -1) {
    zip_source_free(zsource);
    zip_unchange_all(p_archive->archive);
    zip_unchange_archive(p_archive->archive);
    rb_raise(Error, "Add file failed - %s: %s", StringValuePtr(name), zip_strerror(p_archive->archive));
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_replace_file(int argc, VALUE* argv, VALUE self) {
  struct zipruby_archive *p_archive;
  struct zip_source *zsource;
  VALUE index, fname, flags;
  int i_index, i_flags = 0;

  rb_scan_args(argc, argv, "21", &index, &fname, &flags);

  if (!rb_obj_is_instance_of(index, rb_cString) && !rb_obj_is_instance_of(index, rb_cFixnum)) {
    rb_raise(rb_eTypeError, "wrong argument type %s (expected Fixnum or String)", rb_class2name(CLASS_OF(index)));
  }

  if (!NIL_P(flags)) {
    i_flags = NUM2INT(flags);
  }

  Check_Type(fname, T_STRING);
  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  if (rb_obj_is_instance_of(index, rb_cFixnum)) {
    i_index = NUM2INT(index);
  } else if ((i_index = zip_name_locate(p_archive->archive, StringValuePtr(index), i_flags)) == -1) {
    rb_raise(Error, "Replace file failed - %s: Archive does not contain a file", StringValuePtr(index));
  }

  if ((zsource = zip_source_file(p_archive->archive, StringValuePtr(fname), 0, -1)) == NULL) {
    rb_raise(Error, "Replace file failed at %d: %s", i_index, zip_strerror(p_archive->archive));
  }

  if (zip_replace(p_archive->archive, i_index, zsource) == -1) {
    zip_source_free(zsource);
    zip_unchange_all(p_archive->archive);
    zip_unchange_archive(p_archive->archive);
    rb_raise(Error, "Replace file failed at %d: %s", i_index, zip_strerror(p_archive->archive));
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_add_or_replace_file(int argc, VALUE *argv, VALUE self) {
  VALUE name, fname, flags;
  struct zipruby_archive *p_archive;
  int index, i_flags = 0;

  rb_scan_args(argc, argv, "12", &name, &fname, &flags);

  if (NIL_P(flags) && FIXNUM_P(fname)) {
    flags = fname;
    fname = Qnil;
  }

  if (NIL_P(fname)) {
    fname = name;
    name = Qnil;
  }

  Check_Type(fname, T_STRING);

  if (NIL_P(name)) {
    name = rb_funcall(rb_cFile, rb_intern("basename"), 1, fname);
  }

  if (!NIL_P(flags)) {
    i_flags = NUM2INT(flags);
  }

  Check_Type(name, T_STRING);
  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  index = zip_name_locate(p_archive->archive, StringValuePtr(name), i_flags);

  if (index >= 0) {
    VALUE _args[] = { INT2NUM(index), fname };
    return zipruby_archive_replace_file(2, _args, self);
  } else {
    VALUE _args[] = { name, fname };
    return zipruby_archive_add_file(2, _args, self);
  }
}

/* */
static VALUE zipruby_archive_add_filep(int argc, VALUE *argv, VALUE self) {
  VALUE name, file, source;

  rb_scan_args(argc, argv, "11", &name, &file);

  if (NIL_P(file)) {
    file = name;
    name = Qnil;
  }

  Check_Type(file, T_FILE);

  if (NIL_P(name)) {
    name = rb_funcall(rb_cFile, rb_intern("basename"), 1, rb_funcall(file, rb_intern("path"), 0));
  }

  source = rb_funcall(file, rb_intern("read"), 0);

  return zipruby_archive_add_buffer(self, name,  source);
}

/* */
static VALUE zipruby_archive_replace_filep(VALUE self, VALUE index, VALUE file) {
  VALUE source;
  VALUE _args[2];

  Check_Type(file, T_FILE);
  source = rb_funcall(file, rb_intern("read"), 0);

  _args[0] = index;
  _args[1] = source;
  return zipruby_archive_replace_buffer(2, _args, self);
}

/* */
static VALUE zipruby_archive_add_or_replace_filep(int argc, VALUE *argv, VALUE self) {
  VALUE name, file, flags;
  struct zipruby_archive *p_archive;
  int index, i_flags = 0;

  rb_scan_args(argc, argv, "12", &name, &file, &flags);

  if (NIL_P(flags) && FIXNUM_P(file)) {
    flags = file;
    file = Qnil;
  }

  if (NIL_P(file)) {
    file = name;
    name = Qnil;
  }

  Check_Type(file, T_FILE);

  if (NIL_P(name)) {
    name = rb_funcall(rb_cFile, rb_intern("basename"), 1, rb_funcall(file, rb_intern("path"), 0));
  }

  if (!NIL_P(flags)) {
    i_flags = NUM2INT(flags);
  }

  Check_Type(name, T_STRING);
  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  index = zip_name_locate(p_archive->archive, StringValuePtr(name), i_flags);

  if (index >= 0) {
    return zipruby_archive_replace_filep(self, INT2NUM(index), file);
  } else {
    VALUE _args[] = { name, file };
    return zipruby_archive_add_filep(2, _args, self);
  }
}

/* */
static VALUE zipruby_archive_add_function(int argc, VALUE *argv, VALUE self) {
  VALUE name, mtime;
  struct zipruby_archive *p_archive;
  struct zip_source *zsource;
  struct read_proc *z;

  rb_scan_args(argc, argv, "11", &name, &mtime);
  rb_need_block();

  if (NIL_P(mtime)) {
    mtime = rb_funcall(rb_cTime, rb_intern("now"), 0);
  } else if (!rb_obj_is_instance_of(mtime, rb_cTime)) {
    rb_raise(rb_eTypeError, "wrong argument type %s (expected Time)", rb_class2name(CLASS_OF(mtime)));
  }

  Data_Get_Struct(self, struct zipruby_archive, p_archive); 
  Check_Archive(p_archive);

  if ((z = malloc(sizeof(struct read_proc))) == NULL) {
    zip_unchange_all(p_archive->archive);
    zip_unchange_archive(p_archive->archive);
    rb_raise(rb_eRuntimeError, "Add failed - %s: Cannot allocate memory", StringValuePtr(name));
  }

  z->proc = rb_block_proc();
  z->mtime = mtime;

  if ((zsource = zip_source_proc(p_archive->archive, z)) == NULL) {
    free(z);
    rb_raise(Error, "Add failed - %s: %s", StringValuePtr(name), zip_strerror(p_archive->archive));
  }

  if (zip_add(p_archive->archive, StringValuePtr(name), zsource) == -1) {
    zip_source_free(zsource);
    zip_unchange_all(p_archive->archive);
    zip_unchange_archive(p_archive->archive);
    rb_raise(Error, "Add file failed - %s: %s", StringValuePtr(name), zip_strerror(p_archive->archive));
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_replace_function(int argc, VALUE *argv, VALUE self) {
  VALUE index, mtime;
  struct zipruby_archive *p_archive;
  struct zip_source *zsource;
  struct read_proc *z;

  rb_scan_args(argc, argv, "11", &index, &mtime);
  rb_need_block();
  Check_Type(index, T_FIXNUM);

  if (NIL_P(mtime)) {
    mtime = rb_funcall(rb_cTime, rb_intern("now"), 0);
  } else if (!rb_obj_is_instance_of(mtime, rb_cTime)) {
    rb_raise(rb_eTypeError, "wrong argument type %s (expected Time)", rb_class2name(CLASS_OF(mtime)));
  }

  Data_Get_Struct(self, struct zipruby_archive, p_archive); 
  Check_Archive(p_archive);

  if ((z = malloc(sizeof(struct read_proc))) == NULL) {
    zip_unchange_all(p_archive->archive);
    zip_unchange_archive(p_archive->archive);
    rb_raise(rb_eRuntimeError, "Replace failed at %d: Cannot allocate memory", NUM2INT(index));
  }

  z->proc = rb_block_proc();
  z->mtime = mtime;

  if ((zsource = zip_source_proc(p_archive->archive, z)) == NULL) {
    free(z);
    rb_raise(Error, "Replace failed at %d: %s", NUM2INT(index), zip_strerror(p_archive->archive));
  }

  if (zip_replace(p_archive->archive, NUM2INT(index), zsource) == -1) {
    zip_source_free(zsource);
    zip_unchange_all(p_archive->archive);
    zip_unchange_archive(p_archive->archive);
    rb_raise(Error, "Replace failed at %d: %s", NUM2INT(index), zip_strerror(p_archive->archive));
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_add_or_replace_function(int argc, VALUE *argv, VALUE self) {
  VALUE name, mtime, flags;
  struct zipruby_archive *p_archive;
  int index, i_flags = 0;

  rb_scan_args(argc, argv, "12", &name, &mtime, &flags);

  if (NIL_P(flags) && FIXNUM_P(mtime)) {
    flags = mtime;
    mtime = Qnil;
  }

  if (!NIL_P(flags)) {
    i_flags = NUM2INT(flags);
  }

  Check_Type(name, T_STRING);
  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  index = zip_name_locate(p_archive->archive, StringValuePtr(name), i_flags);

  if (index >= 0) {
    VALUE _args[] = { INT2NUM(index), mtime };
    return zipruby_archive_replace_function(2, _args, self);
  } else {
    VALUE _args[] = { name, mtime };
    return zipruby_archive_add_function(2, _args, self);
  }
}

/* */
static VALUE zipruby_archive_update(int argc, VALUE *argv, VALUE self) {
  struct zipruby_archive *p_archive, *p_srcarchive;
  VALUE srcarchive, flags;
  int i, num_files, i_flags = 0;

  rb_scan_args(argc, argv, "11", &srcarchive, &flags);

  if (!rb_obj_is_instance_of(srcarchive, Archive)) {
    rb_raise(rb_eTypeError, "wrong argument type %s (expected Zip::Archive)", rb_class2name(CLASS_OF(srcarchive)));
  }

  if (!NIL_P(flags)) {
    i_flags = NUM2INT(flags);
  }

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);
  Data_Get_Struct(srcarchive, struct zipruby_archive, p_srcarchive);
  Check_Archive(p_srcarchive);

  num_files = zip_get_num_files(p_srcarchive->archive);

  for (i = 0; i < num_files; i++) {
    struct zip_source *zsource;
    struct zip_file *fzip;
    struct zip_stat sb;
    char *buf;
    const char *name;
    int index, error;

    zip_stat_init(&sb);

    if (zip_stat_index(p_srcarchive->archive, i, 0, &sb)) {
      zip_unchange_all(p_archive->archive);
      zip_unchange_archive(p_archive->archive);
      rb_raise(Error, "Update archive failed: %s", zip_strerror(p_srcarchive->archive));
    }

    if ((buf = malloc(sb.size)) == NULL) {
      zip_unchange_all(p_archive->archive);
      zip_unchange_archive(p_archive->archive);
      rb_raise(rb_eRuntimeError, "Update archive failed: Cannot allocate memory");
    }

    fzip = zip_fopen_index(p_srcarchive->archive, i, 0);

    if (fzip == NULL) {
      free(buf);
      zip_unchange_all(p_archive->archive);
      zip_unchange_archive(p_archive->archive);
      rb_raise(Error, "Update archive failed: %s", zip_strerror(p_srcarchive->archive));
    }

    if (zip_fread(fzip, buf, sb.size) == -1) {
      free(buf);
      zip_fclose(fzip);
      zip_unchange_all(p_archive->archive);
      zip_unchange_archive(p_archive->archive);
      rb_raise(Error, "Update archive failed: %s", zip_file_strerror(fzip));
    }

    if ((error = zip_fclose(fzip)) != 0) {
      char errstr[ERRSTR_BUFSIZE];
      free(buf);
      zip_unchange_all(p_archive->archive);
      zip_unchange_archive(p_archive->archive);
      zip_error_to_str(errstr, ERRSTR_BUFSIZE, error, errno);
      rb_raise(Error, "Update archive failed: %s", errstr);
    }

    if ((zsource = zip_source_buffer(p_archive->archive, buf, sb.size, 1)) == NULL) {
      free(buf);
      zip_unchange_all(p_archive->archive);
      zip_unchange_archive(p_archive->archive);
      rb_raise(Error, "Update archive failed: %s", zip_strerror(p_archive->archive));
    }

    if ((name = zip_get_name(p_srcarchive->archive, i, 0)) == NULL) {
      zip_source_free(zsource);
      zip_unchange_all(p_archive->archive);
      zip_unchange_archive(p_archive->archive);
      rb_raise(Error, "Update archive failed: %s", zip_strerror(p_srcarchive->archive));
    }

    index = zip_name_locate(p_archive->archive, name, i_flags);

    if (index >= 0) {
      if (zip_replace(p_archive->archive, i, zsource) == -1) {
        zip_source_free(zsource);
        zip_unchange_all(p_archive->archive);
        zip_unchange_archive(p_archive->archive);
        rb_raise(Error, "Update archive failed: %s", zip_strerror(p_archive->archive));
      }
    } else {
      if (zip_add(p_archive->archive, name, zsource) == -1) {
        zip_source_free(zsource);
        zip_unchange_all(p_archive->archive);
        zip_unchange_archive(p_archive->archive);
        rb_raise(Error, "Update archive failed: %s", zip_strerror(p_archive->archive));
      }
    }
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_get_comment(int argc, VALUE *argv, VALUE self) {
  VALUE flags;
  struct zipruby_archive *p_archive;
  const char *comment;
  int lenp, i_flags = 0;

  rb_scan_args(argc, argv, "01", &flags);

  if (!NIL_P(flags)) {
    i_flags = NUM2INT(flags);
  }

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  // XXX: How is the error checked?
  comment = zip_get_archive_comment(p_archive->archive, &lenp, i_flags);

  return comment ? rb_str_new(comment, lenp) : Qnil;
}

/* */
static VALUE zipruby_archive_set_comment(VALUE self, VALUE comment) {
  struct zipruby_archive *p_archive;
  const char *s_comment = NULL;
  int len = 0;

  if (!NIL_P(comment)) {
    Check_Type(comment, T_STRING);
    s_comment = StringValuePtr(comment);
    len = RSTRING(comment)->len;
  }

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  if (zip_set_archive_comment(p_archive->archive, s_comment, len) == -1) {
    zip_unchange_all(p_archive->archive);
    zip_unchange_archive(p_archive->archive);
    rb_raise(Error, "Comment archived failed: %s", zip_strerror(p_archive->archive));
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_locate_name(int argc, VALUE *argv, VALUE self) {
  VALUE fname, flags;
  struct zipruby_archive *p_archive;
  int i_flags = 0;

  rb_scan_args(argc, argv, "11", &fname, &flags);
  Check_Type(fname, T_STRING);

  if (!NIL_P(flags)) {
    i_flags = NUM2INT(flags);
  }

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  return INT2NUM(zip_name_locate(p_archive->archive, StringValuePtr(fname), i_flags));
}

/* */
static VALUE zipruby_archive_get_fcomment(int argc, VALUE *argv, VALUE self) {
  VALUE index, flags;
  struct zipruby_archive *p_archive;
  const char *comment;
  int lenp, i_flags = 0;

  rb_scan_args(argc, argv, "11", &index, &flags);

  if (!NIL_P(flags)) {
    i_flags = NUM2INT(flags);
  }

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  // XXX: How is the error checked?
  comment = zip_get_file_comment(p_archive->archive, NUM2INT(index), &lenp, i_flags);

  return comment ? rb_str_new(comment, lenp) : Qnil;
}

/* */
static VALUE zipruby_archive_set_fcomment(VALUE self, VALUE index, VALUE comment) {
  struct zipruby_archive *p_archive;
  char *s_comment = NULL;
  int len = 0;

  if (!NIL_P(comment)) {
    Check_Type(comment, T_STRING);
    s_comment = StringValuePtr(comment);
    len = RSTRING(comment)->len;
  }

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  if (zip_set_file_comment(p_archive->archive, NUM2INT(index), s_comment, len) == -1) {
    zip_unchange_all(p_archive->archive);
    zip_unchange_archive(p_archive->archive);
    rb_raise(Error, "Comment file failed at %d: %s", NUM2INT(index), zip_strerror(p_archive->archive));
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_fdelete(VALUE self, VALUE index) {
  struct zipruby_archive *p_archive;

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  if (zip_delete(p_archive->archive, NUM2INT(index)) == -1) {
    zip_unchange_all(p_archive->archive);
    zip_unchange_archive(p_archive->archive);
    rb_raise(Error, "Delete file failed at %d: %s", NUM2INT(index), zip_strerror(p_archive->archive));
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_frename(VALUE self, VALUE index, VALUE name) {
  struct zipruby_archive *p_archive;

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  if (zip_rename(p_archive->archive, NUM2INT(index), StringValuePtr(name)) == -1) {
    zip_unchange_all(p_archive->archive);
    zip_unchange_archive(p_archive->archive);
    rb_raise(Error, "Rename file failed at %d: %s", NUM2INT(index), zip_strerror(p_archive->archive));
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_funchange(VALUE self, VALUE index) {
  struct zipruby_archive *p_archive;

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  if (zip_unchange(p_archive->archive, NUM2INT(index)) == -1) {
    zip_unchange_all(p_archive->archive);
    zip_unchange_archive(p_archive->archive);
    rb_raise(Error, "Unchange file failed at %d: %s", NUM2INT(index), zip_strerror(p_archive->archive));
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_funchange_all(VALUE self) {
  struct zipruby_archive *p_archive;

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  if (zip_unchange_all(p_archive->archive) == -1) {
    rb_raise(Error, "Unchange all file failed: %s", zip_strerror(p_archive->archive));
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_unchange(VALUE self) {
  struct zipruby_archive *p_archive;

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);

  if (zip_unchange_archive(p_archive->archive) == -1) {
    rb_raise(Error, "Unchange archive failed: %s", zip_strerror(p_archive->archive));
  }

  return Qnil;
}

/* */
static VALUE zipruby_archive_revert(VALUE self) {
  zipruby_archive_funchange_all(self);
  zipruby_archive_unchange(self);

  return Qnil;
}

/* */
static VALUE zipruby_archive_each(VALUE self) {
  struct zipruby_archive *p_archive;
  int i, num_files;

  Data_Get_Struct(self, struct zipruby_archive, p_archive);
  Check_Archive(p_archive);
  num_files = zip_get_num_files(p_archive->archive);

  for (i = 0; i < num_files; i++) {
    VALUE file;
    int status;

    file = rb_funcall(File, rb_intern("new"), 2, self, INT2NUM(i));
    rb_protect(rb_yield, file, &status);
    rb_funcall(file, rb_intern("close"), 0);

    if (status != 0) {
      rb_jump_tag(status);
    }
  }

  return Qnil;
}
