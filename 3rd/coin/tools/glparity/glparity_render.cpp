#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <Inventor/SbColor.h>
#include <Inventor/SbVec2s.h>
#include <Inventor/SbVec3f.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoPath.h>
#include <Inventor/SoSceneManager.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoSeparator.h>

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>

namespace {

void
usage(const char * program)
{
  std::cerr
    << "Usage: " << program
    << " --scene <absolute-or-relative.iv> --output <directory>\n"
    << "       [--width 400] [--height 300] [--allow-empty]\n";
}

bool
parseDimension(const std::string & text, int & value)
{
  char * end = NULL;
  errno = 0;
  const long parsed = std::strtol(text.c_str(), &end, 10);
  if (errno != 0 || end == text.c_str() || *end != '\0' ||
      parsed <= 0 || parsed > 32767) {
    return false;
  }
  value = static_cast<int>(parsed);
  return true;
}

bool
makeDirectories(const std::string & path)
{
  if (path.empty()) return false;

  std::string current;
  if (path[0] == '/') current = "/";

  std::string::size_type begin = path[0] == '/' ? 1 : 0;
  while (begin <= path.size()) {
    const std::string::size_type end = path.find('/', begin);
    const std::string component =
      path.substr(begin, end == std::string::npos ? end : end - begin);
    if (!component.empty()) {
      if (!current.empty() && current[current.size() - 1] != '/') current += '/';
      current += component;
      if (mkdir(current.c_str(), 0777) != 0 && errno != EEXIST) {
        std::cerr << "Could not create output directory '" << current
                  << "': " << std::strerror(errno) << "\n";
        return false;
      }
    }
    if (end == std::string::npos) break;
    begin = end + 1;
  }
  return true;
}

std::string
jsonEscape(const std::string & text)
{
  std::ostringstream escaped;
  for (std::string::const_iterator it = text.begin(); it != text.end(); ++it) {
    const unsigned char c = static_cast<unsigned char>(*it);
    switch (c) {
    case '"': escaped << "\\\""; break;
    case '\\': escaped << "\\\\"; break;
    case '\b': escaped << "\\b"; break;
    case '\f': escaped << "\\f"; break;
    case '\n': escaped << "\\n"; break;
    case '\r': escaped << "\\r"; break;
    case '\t': escaped << "\\t"; break;
    default:
      if (c < 0x20) {
        escaped << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                << static_cast<unsigned int>(c) << std::dec;
      }
      else {
        escaped << static_cast<char>(c);
      }
      break;
    }
  }
  return escaped.str();
}

const char *
profileName()
{
#if defined(COIN_GLES3)
  return "GLES3";
#elif defined(COIN_GLES2)
  return "GLES2";
#elif defined(COIN_GL3_CORE)
  return "GL3";
#else
  return "COMPAT";
#endif
}

} // namespace

int
main(int argc, char ** argv)
{
  std::string scene;
  std::string output;
  int width = 400;
  int height = 300;
  bool allowempty = false;

  for (int i = 1; i < argc; ++i) {
    const std::string argument(argv[i]);
    if (argument == "--allow-empty") {
      allowempty = true;
      continue;
    }
    if (argument == "--scene" || argument == "--output" ||
        argument == "--width" || argument == "--height") {
      if (i + 1 >= argc) {
        usage(argv[0]);
        return 2;
      }
      const std::string value(argv[++i]);
      if (argument == "--scene") {
        scene = value;
      }
      else if (argument == "--output") {
        output = value;
      }
      else if (argument == "--width" && !parseDimension(value, width)) {
        std::cerr << "Invalid width: " << value << "\n";
        return 2;
      }
      else if (argument == "--height" && !parseDimension(value, height)) {
        std::cerr << "Invalid height: " << value << "\n";
        return 2;
      }
      continue;
    }

    usage(argv[0]);
    return 2;
  }

  if (scene.empty() || output.empty()) {
    usage(argv[0]);
    return 2;
  }

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL initialization failed: " << SDL_GetError() << "\n";
    return 1;
  }

#if defined(COIN_GLES3)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(COIN_GLES2)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(COIN_GL3_CORE)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                      SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

  SDL_Window * window = SDL_CreateWindow(
    "Coin glparity renderer", width, height,
    SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
  if (!window) {
    std::cerr << "SDL window creation failed: " << SDL_GetError() << "\n";
    SDL_Quit();
    return 1;
  }

  SDL_GLContext context = SDL_GL_CreateContext(window);
  if (!context) {
    std::cerr << "OpenGL context creation failed: " << SDL_GetError() << "\n";
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  if (!SDL_GL_MakeCurrent(window, context)) {
    std::cerr << "Could not make OpenGL context current: " << SDL_GetError()
              << "\n";
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  while (glGetError() != GL_NO_ERROR) { }
  glEnable(GL_DEPTH_TEST);

  const GLubyte * versiontext = glGetString(GL_VERSION);
  const std::string glversion =
    versiontext ? reinterpret_cast<const char *>(versiontext) : "";

  SoDB::init();
  SoInput input;
  if (!input.openFile(scene.c_str())) {
    std::cerr << "Could not open scene: " << scene << "\n";
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  SoSeparator * loadedscene = SoDB::readAll(&input);
  input.closeFile();
  if (!loadedscene) {
    std::cerr << "Could not read scene: " << scene << "\n";
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }
  loadedscene->ref();

  SoSearchAction search;
  search.setType(SoCamera::getClassTypeId(), TRUE);
  search.setInterest(SoSearchAction::FIRST);
  search.apply(loadedscene);
  SoCamera * camera = search.getPath()
    ? static_cast<SoCamera *>(search.getPath()->getTail())
    : NULL;

  SoSeparator * harness = new SoSeparator;
  harness->ref();
  SoPerspectiveCamera * insertedcamera = NULL;
  if (!camera) {
    insertedcamera = new SoPerspectiveCamera;
    camera = insertedcamera;
    harness->addChild(camera);
  }

  SoDirectionalLight * light = new SoDirectionalLight;
  light->direction = SbVec3f(0.35f, -1.0f, -0.6f);
  light->intensity = 1.0f;
  harness->addChild(light);
  harness->addChild(loadedscene);
  loadedscene->unref();

  SoSceneManager * manager = new SoSceneManager;
  manager->setBackgroundColor(SbColor(0.12f, 0.14f, 0.18f));
  manager->setWindowSize(SbVec2s(static_cast<short>(width),
                                 static_cast<short>(height)));
  manager->activate();
  manager->setSceneGraph(harness);

  if (insertedcamera) {
    camera->viewAll(harness, manager->getViewportRegion());
    const float dist = std::fabs(camera->position.getValue()[2]);
    camera->nearDistance = dist * 0.1f;
    camera->farDistance = dist * 10.0f;
  }

  manager->render();
  glFinish();

  std::vector<unsigned char> pixels(
    static_cast<size_t>(width) * static_cast<size_t>(height) * 4u);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);

  const unsigned char clear[3] = {31, 36, 46};
  size_t nonbackground = 0;
  for (size_t i = 0; i < pixels.size(); i += 4) {
    const int r = pixels[i];
    const int g = pixels[i + 1];
    const int b = pixels[i + 2];
    if (std::abs(r - static_cast<int>(clear[0])) > 8 ||
        std::abs(g - static_cast<int>(clear[1])) > 8 ||
        std::abs(b - static_cast<int>(clear[2])) > 8) {
      ++nonbackground;
    }
  }

  bool outputok = makeDirectories(output);
  if (outputok) {
    std::ofstream raw((output + "/frame0000.rgba").c_str(),
                      std::ios::out | std::ios::binary);
    raw.write(reinterpret_cast<const char *>(&pixels[0]),
              static_cast<std::streamsize>(pixels.size()));
    outputok = raw.good();
    if (!outputok) {
      std::cerr << "Could not write raw frame output\n";
    }
  }

  const size_t center =
    (static_cast<size_t>(height / 2) * static_cast<size_t>(width) +
     static_cast<size_t>(width / 2)) * 4u;
  const double ratio = static_cast<double>(nonbackground) /
                       static_cast<double>(width * height);
  if (outputok) {
    std::ofstream stats((output + "/stats.json").c_str());
    stats << "{\n"
          << "  \"width\": " << width << ",\n"
          << "  \"height\": " << height << ",\n"
          << "  \"non_bg_pixels\": " << nonbackground << ",\n"
          << "  \"non_bg_ratio\": " << std::setprecision(10) << ratio << ",\n"
          << "  \"center_rgba\": ["
          << static_cast<unsigned int>(pixels[center]) << ", "
          << static_cast<unsigned int>(pixels[center + 1]) << ", "
          << static_cast<unsigned int>(pixels[center + 2]) << ", "
          << static_cast<unsigned int>(pixels[center + 3]) << "],\n"
          << "  \"gl_version\": \"" << jsonEscape(glversion) << "\",\n"
          << "  \"profile\": \"" << profileName() << "\",\n"
          << "  \"bg_rgb\": [31, 36, 46]\n"
          << "}\n";
    outputok = stats.good();
    if (!outputok) {
      std::cerr << "Could not write frame statistics\n";
    }
  }

  harness->unref();
  delete manager;
  SDL_GL_DestroyContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  if (!outputok) return 1;
  if (nonbackground == 0 && !allowempty) {
    std::cerr << "Rendered frame has zero non-background coverage"
              << " (profile=" << profileName() << " scene=" << scene << ")\n";
    return 1;
  }
  return 0;
}
