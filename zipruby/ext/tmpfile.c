#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "tmpfile.h"

#ifndef _WIN32
#ifndef HAVE_MKSTEMP
int _zip_mkstemp(char *);
#define mkstemp _zip_mkstemp
#endif
#endif

char *zipruby_tmpnum(void *data, int len) {
  char *filnum;

#ifdef _WIN32
  char tmpdirnum[_MAX_PATH];
  char tmpfilnum[_MAX_PATH];
  int numlen;

  memset(tmpdirnum, 0, _MAX_PATH);

  if (GetTempPathA(_MAX_PATH, tmpdirnum) == 0) {
    return NULL;
  }

  memset(tmpfilnum, 0, _MAX_PATH);

  if (GetTempFileNameA(tmpdirnum, "zrb", 0, tmpfilnum) == 0) {
    return NULL;
  }

  numlen = strlen(tmpfilnum) + 1;
  filnum = calloc(len, sizeof(char));

  if (strcpy_s(filnum, numlen, tmpfilnum) != 0) {
    free(filnum);
    return NULL;
  }
#else
  int fd;
#ifdef P_tmpdir
  static int numlen = 16 + strlen(P_tmpdir);
  char *dirnum = P_tmpdir;
#else
  int numlen = 20;
  char *dirnum = "/tmp";
#endif

  filnum = calloc(numlen, sizeof(char));
  strcpy(filnum, dirnum);
  strcat(filnum, "/zipruby.XXXXXX");

  if ((fd = mkstemp(filnum)) == -1) {
    free(filnum);
    return NULL;
  }

  if (close(fd) == -1) {
    free(filnum);
    return NULL;
  }
#endif

  return filnum;
}
