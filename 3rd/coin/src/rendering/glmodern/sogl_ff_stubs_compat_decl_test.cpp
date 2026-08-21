/* Compile-only: Linux <GL/gl.h> declares fixed-function APIs as extern
   and defines compat tokens such as GL_LIGHTING. Stubs must not redeclare
   those entry points as static, and must not #define them (breaks w->glXxx). */

#define GL_LIGHTING 0x0B50
#define GL_MODELVIEW 0x1700

typedef unsigned int GLenum;

#ifdef __cplusplus
extern "C" {
#endif
void glPrioritizeTextures(int, const unsigned int *, const float *) {}
unsigned char glAreTexturesResident(int, const unsigned int *, unsigned char *) { return 0; }
void glBegin(GLenum) {}
void glEnd(void) {}
#ifdef __cplusplus
}
#endif

#include <Inventor/system/sogl_ff_stubs.h>

struct cc_glglue_probe {
  void (*glPushClientAttrib)(unsigned int);
};

static void
call_glue_member(cc_glglue_probe * w, unsigned int mask)
{
  w->glPushClientAttrib(mask);
}

int
main(void)
{
  cc_glglue_probe glue;
  glue.glPushClientAttrib = 0;
  if (glue.glPushClientAttrib) {
    call_glue_member(&glue, 0);
  }
  glPrioritizeTextures(0, 0, 0);
  (void)glAreTexturesResident(0, 0, 0);
  glBegin(0);
  glEnd();
  /* GLES2 headers omit these pixel-format enums; NV combiner code uses GL_BLUE. */
  if (GL_RED != 0x1903 || GL_GREEN != 0x1904 || GL_BLUE != 0x1905) {
    return 1;
  }
  return 0;
}
