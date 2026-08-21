/* Compile-only: GLES2/GLES3 headers do not declare GLdouble. Coin glue
   still uses it in public prototypes (e.g. Inventor/C/glue/gl.h). This
   translation unit simulates ES headers (no GL_DOUBLE / GLdouble) and
   checks Inventor/system/sogl_es_types.h provides the typedef. */

#include <Inventor/system/sogl_es_types.h>

int
main(void)
{
  GLdouble x = 0.0;
  GLdouble params[4] = { 0.0, 0.0, 0.0, 0.0 };
  (void)x;
  (void)params;
  return (GL_DOUBLE == 0x140A) ? 0 : 1;
}
