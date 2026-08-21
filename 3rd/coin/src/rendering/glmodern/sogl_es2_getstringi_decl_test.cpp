/* Compile-only: GLES2 headers do not declare glGetStringi. Keep this #if
   in sync with cc_glglue_instance() in glue/gl.cpp — the linked-symbol
   fallback is Core / ES3 only. */

#define COIN_GL_MODERN 1
#define COIN_GLES2 1

typedef void *(*coin_getstringi_proc)(unsigned int, unsigned int);

int
main(void)
{
  coin_getstringi_proc pGetStringi = 0;
#if (defined(COIN_GL3_CORE) || defined(COIN_GLES3)) && !defined(COIN_USE_GLAD)
  pGetStringi = (coin_getstringi_proc)glGetStringi;
#endif
  (void)pGetStringi;
  return 0;
}
