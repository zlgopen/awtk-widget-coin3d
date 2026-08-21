/* Parse glGetString(GL_VERSION). Desktop is "1.5.0 vendor"; ES is
   "OpenGL ES 2.0 Mesa ...". Skip any non-digit prefix so ES does not
   parse as 0.0. */

#ifndef COIN_SOGL_GL_VERSION_H
#define COIN_SOGL_GL_VERSION_H

#include <stdlib.h>
#include <string.h>

static inline void
sogl_parse_gl_version_string(const char * versionstr,
                             int * major, int * minor, int * release)
{
  char buffer[256];
  const char * src;
  char * dotptr;
  char * start;

  *major = 0;
  *minor = 0;
  *release = 0;
  if (versionstr == NULL) return;

  src = versionstr;
  while (*src && (*src < '0' || *src > '9')) {
    ++src;
  }

  (void)strncpy(buffer, src, 255);
  buffer[255] = '\0';
  dotptr = strchr(buffer, '.');
  if (dotptr) {
    char * spaceptr;
    start = buffer;
    *dotptr = '\0';
    *major = atoi(start);
    start = ++dotptr;

    dotptr = strchr(start, '.');
    spaceptr = strchr(start, ' ');
    if (!dotptr && spaceptr) dotptr = spaceptr;
    if (dotptr && spaceptr && spaceptr < dotptr) dotptr = spaceptr;
    if (dotptr) {
      int terminate = *dotptr == ' ';
      *dotptr = '\0';
      *minor = atoi(start);
      if (!terminate) {
        start = ++dotptr;
        dotptr = strchr(start, ' ');
        if (dotptr) *dotptr = '\0';
        *release = atoi(start);
      }
    }
    else {
      *minor = atoi(start);
    }
  }
}

#endif /* ! COIN_SOGL_GL_VERSION_H */
