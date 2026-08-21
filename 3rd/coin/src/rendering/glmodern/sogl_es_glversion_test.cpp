/* GLES version strings start with "OpenGL ES "; desktop strings start
   with the number. Glue used to atoi() from the first character and
   treat ES as 0.0, so VBO symbols were never resolved. */

#include <Inventor/system/sogl_gl_version.h>

int
main(void)
{
  int major = -1;
  int minor = -1;
  int release = -1;

  sogl_parse_gl_version_string("OpenGL ES 2.0 Mesa 23.2.1", &major, &minor, &release);
  if (major != 2 || minor != 0) {
    return 1;
  }

  sogl_parse_gl_version_string("OpenGL ES 3.1 Mesa 23.2.1", &major, &minor, &release);
  if (major != 3 || minor != 1) {
    return 1;
  }

  sogl_parse_gl_version_string("1.5.0 NVIDIA 340.108", &major, &minor, &release);
  if (major != 1 || minor != 5 || release != 0) {
    return 1;
  }

  sogl_parse_gl_version_string("4.5.0 NVIDIA", &major, &minor, &release);
  if (major != 4 || minor != 5) {
    return 1;
  }

  return 0;
}
