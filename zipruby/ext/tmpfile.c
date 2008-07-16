#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#endif

#include "tmpfile.h"

#ifndef _WIN32
#ifndef HAVE_MKSTEMP
int _zip_mkstemp(char *);
#define mkstemp _zip_mkstemp
#endif
#endif

char *zipruby_tmpnam(void *data, int len) {
  char *filnam;

#ifdef _WIN32
  int fd;
  char tmpdirnam[_MAX_PATH];
  char tmpfilnam[_MAX_PATH];
  int namlen;

  memset(tmpdirnam, 0, _MAX_PATH);

  if (GetTempPathA(_MAX_PATH, tmpdirnam) == 0) {
    return NULL;
  }

  memset(tmpfilnam, 0, _MAX_PATH);

  if (GetTempFileNameA(tmpdirnam, "zrb", 0, tmpfilnam) == 0) {
    return NULL;
  }

  namlen = strlen(tmpfilnam) + 1;
  filnam = calloc(namlen, sizeof(char));

  if (strcpy_s(filnam, namlen, tmpfilnam) != 0) {
    free(filnam);
    return NULL;
  }

  if (data) {
    if ((_sopen_s(&fd, filnam, _O_WRONLY | _O_BINARY, _SH_DENYRD, _S_IWRITE)) != 0) {
      free(filnam);
      return NULL;
    }

    if (_write(fd, data, len) == -1) {
      free(filnam);
      return NULL;
    }

    if (_close(fd) == -1) {
      free(filnam);
      return NULL;
    }
  }
#else
  int fd;
#ifdef P_tmpdir
  int namlen = 16 + strlen(P_tmpdir);
  char *dirnam = P_tmpdir;
#else
  int namlen = 20;
  char *dirnam = "/tmp";
#endif

  filnam = calloc(namlen, sizeof(char));
  strcpy(filnam, dirnam);
  strcat(filnam, "/zipruby.XXXXXX");

  if ((fd = mkstemp(filnam)) == -1) {
    free(filnam);
    return NULL;
  }

  if (data) {
    if (write(fd, data, len) == -1) {
      free(filnam);
      return NULL;
    }
  }

  if (close(fd) == -1) {
    free(filnam);
    return NULL;
  }
#endif

  return filnam;
}

void zipruby_rmtmp(const char *tmpfilnam) {
  struct stat st;

  if (!tmpfilnam) {
    return;
  }

  if (stat(tmpfilnam, &st) != 0) {
    return;
  }

#ifdef _WIN32
  _unlink(tmpfilnam);
#else
  unlink(tmpfilnam);
#endif
}
