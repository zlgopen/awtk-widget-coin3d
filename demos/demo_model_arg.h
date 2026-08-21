/**
 * File:   demo_model_arg.h
 * Author: AWTK Develop Team
 * Brief:  parse .iv path from command line
 *
 * Copyright (c) 2026 - 2026 Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 */

#ifndef DEMO_MODEL_ARG_H
#define DEMO_MODEL_ARG_H

#include "tkc/types_def.h"
#include "tkc/path.h"
#include "tkc/utils.h"

BEGIN_C_DECLS

#ifndef DEMO_IV_EXT
#define DEMO_IV_EXT ".iv"
#endif

#ifndef DEMO_STL_EXT
#define DEMO_STL_EXT ".stl"
#endif

#ifndef DEMO_SCREENSHOT_OPT
#define DEMO_SCREENSHOT_OPT "--screenshot"
#endif

static inline const char* demo_find_model_arg(int argc, char* argv[]) {
  int i = 0;

  if (argv == NULL) {
    return NULL;
  }

  for (i = 1; i < argc; i++) {
    if (argv[i] != NULL && tk_str_eq(argv[i], DEMO_SCREENSHOT_OPT)) {
      i++;
      continue;
    }
    if (argv[i] != NULL &&
        (path_extname_is(argv[i], DEMO_IV_EXT) || path_extname_is(argv[i], DEMO_STL_EXT))) {
      return argv[i];
    }
  }

  return NULL;
}

static inline const char* demo_find_screenshot_arg(int argc, char* argv[]) {
  int i = 0;

  if (argv == NULL) {
    return NULL;
  }

  for (i = 1; i < argc - 1; i++) {
    if (tk_str_eq(argv[i], DEMO_SCREENSHOT_OPT) && argv[i + 1] != NULL && argv[i + 1][0] != '\0') {
      return argv[i + 1];
    }
  }

  return NULL;
}

END_C_DECLS

#endif /*DEMO_MODEL_ARG_H*/
