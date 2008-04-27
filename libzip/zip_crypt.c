#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define close(f) _close(f)
#define rename(s, d) (MoveFileExA((s), (d), MOVEFILE_REPLACE_EXISTING) ? 0 : -1)
#endif

#include "zip.h"
#include "zipint.h"

#include "crc32.h"

#define CRYPT_READ_AHEAD_LEN 12

static void update_keys(unsigned long *keys, char c) {
  keys[0] = CRC32(keys[0], c);
  keys[1] += keys[0] & 0xff;
  keys[1] = keys[1] * 134775813L + 1;
  c = (char) (keys[1] >> 24);
  keys[2] = CRC32(keys[2], c);
}

static int decrypt_byte(unsigned long *keys) {
  unsigned temp;

  temp = (unsigned) (keys[2] | 2);
  return (int) ((temp * (temp ^ 1)) >> 8);
}

static void init_keys(unsigned long *keys, const char *password, size_t len) {
  int i;

  keys[0] = 305419896L;
  keys[1] = 591751049L;
  keys[2] = 878082192L;

  for (i = 0; i < len; i++) {
    update_keys(keys, password[i]);
  }
}

static void decrypt_header(unsigned long *keys, char *buffer) {
  int i;

  for (i = 0; i < CRYPT_READ_AHEAD_LEN; i++) {
    char c = buffer[i] ^ decrypt_byte(keys);
    update_keys(keys, c);
    buffer[i] = c;
  }
}

static void decrypt_data(unsigned long *keys, char *buffer, size_t n) {
  int i;

  for (i = 0; i < n; i++) {
    char temp = buffer[i] ^ decrypt_byte(keys);
    update_keys(keys, temp);
    buffer[i] = temp;
  }
}

static int copy_decrypt(FILE *src, off_t len, const char *password, int passwdlen, FILE *dest, struct zip_error *error) {
  char buf[BUFSIZE];
  unsigned long keys[3];
  int n;
  
  if (len == 0) {
    return 0;
  }

  init_keys(keys, password, passwdlen);

  if (fread(buf, 1, CRYPT_READ_AHEAD_LEN, src) < 0) {
    _zip_error_set(error, ZIP_ER_READ, errno);
  }

  decrypt_header(keys, buf);

  while (len > 0) {
    if ((n = fread(buf, 1, (len > sizeof(buf) ? sizeof(buf) : len), src)) < 0) {
      _zip_error_set(error, ZIP_ER_READ, errno);
      return -1;
    } else if (n == 0) {
      _zip_error_set(error, ZIP_ER_EOF, 0);
      return -1;
    }

    decrypt_data(keys, buf, n);

    if (fwrite(buf, 1, n, dest) != ((size_t) n)) {
      _zip_error_set(error, ZIP_ER_WRITE, errno);
      return -1;
    }

    len -= n;
  }

  return 0;
}

static int _zip_crypt(struct zip *za, const char *password, int passwdlen, int decrypt) {
  int i, j, error;
  char *temp;
  FILE *out;
#ifndef _WIN32
  mode_t mask;
#endif
  struct zip_cdir *cd;
  struct zip_dirent de;
  int reopen_on_error;

  reopen_on_error = 0;

  if (za == NULL) {
    return -1;
  }

  if (za->nentry < 1) {
    _zip_free(za);
    return 0;
  }

  if ((cd = _zip_cdir_new(za->nentry, &za->error)) == NULL) {
    return -1;
  }

  for (i = 0; i < za->nentry; i++) {
    _zip_dirent_init(&cd->entry[i]);
  }

  if (_zip_cdir_set_comment(cd, za) == -1) {
    _zip_cdir_free(cd);
    return -1;
  }

  if ((temp = _zip_create_temp_output(za, &out)) == NULL) {
    _zip_cdir_free(cd);
    return -1;
  }

  error = 0;

  for (i = j = 0; i < za->nentry; i++) {
    int copy_result;
    int encrypted;

    if (fseeko(za->zp, za->cdir->entry[i].offset, SEEK_SET) != 0) {
      _zip_error_set(&za->error, ZIP_ER_SEEK, errno);
      error = 1;
      break;
    }

    if (_zip_dirent_read(&de, za->zp, NULL, 0, 1, &za->error) != 0) {
      error = 1;
      break;
    }

    if (de.bitflags & ZIP_GPBF_DATA_DESCRIPTOR) {
      de.crc = za->cdir->entry[i].crc;
      de.comp_size = za->cdir->entry[i].comp_size;
      de.uncomp_size = za->cdir->entry[i].uncomp_size;
      de.bitflags &= ~ZIP_GPBF_DATA_DESCRIPTOR;
    }

    memcpy(cd->entry + j, za->cdir->entry + i, sizeof(cd->entry[j]));
    cd->entry[j].offset = ftello(out);

    encrypted = (de.bitflags & ZIP_GPBF_ENCRYPTED);

    if (decrypt && encrypted) {
      de.comp_size -= CRYPT_READ_AHEAD_LEN;
      de.bitflags &= ~ZIP_GPBF_ENCRYPTED;
      cd->entry[j].comp_size -= CRYPT_READ_AHEAD_LEN;
      cd->entry[j].bitflags &= ~ZIP_GPBF_ENCRYPTED;
    } else if (!decrypt && !encrypted) {
      // XXX
    }

    if (_zip_dirent_write(&de, out, 1, &za->error) < 0) {
      error = 1;
      break;
    }

    if (decrypt && encrypted) {
      copy_result = copy_decrypt(za->zp, cd->entry[j].comp_size, password, passwdlen, out, &za->error);
    } else if (!decrypt && !encrypted) {
      // XXX
      copy_result = copy_data(za->zp, cd->entry[j].comp_size, out, &za->error);
    } else {
      copy_result = copy_data(za->zp, cd->entry[j].comp_size, out, &za->error);
    }

    if (copy_result < 0) {
      error = 1;
      break;
    }

    j++;
    _zip_dirent_finalize(&de);
  }

  if (!error && _zip_cdir_write(cd, out, &za->error) < 0) {
    error = 1;
  }

  cd->nentry = 0;
  _zip_cdir_free(cd);

  if (error) {
    _zip_dirent_finalize(&de);
    fclose(out);
    remove(temp);
    free(temp);
    return -1;
  }

  if (fclose(out) != 0) {
    _zip_error_set(&za->error, ZIP_ER_CLOSE, errno);
    remove(temp);
    free(temp);
    return -1;
  }

  if (za->zp) {
    fclose(za->zp);
    za->zp = NULL;
    reopen_on_error = 1;
  }

  if (rename(temp, za->zn) != 0) {
    _zip_error_set(&za->error, ZIP_ER_RENAME, errno);
    remove(temp);
    free(temp);

    if (reopen_on_error) {
      za->zp = fopen(za->zn, "rb");
    }

    return -1;
  }

#ifndef _WIN32
  mask = umask(0);
  umask(mask);
  chmod(za->zn, 0666&~mask);
#endif

  free(temp);
  return 0;
}

int zip_decrypt(const char *path, const char *password, int passwdlen, int *errorp) {
  struct zip *za;
  int res;

  if ((za = zip_open(path, 0, errorp)) == NULL) {
    return -1;
  }

  res = _zip_crypt(za, password, passwdlen, 1);
  _zip_free(za);

  return res;
}
