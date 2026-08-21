/* SDL3 + Coin3D manual viewer for COMPAT / GL3 verification.
 *
 * Built-in scenes match tools/glparity/scenes_p0.txt. Pass an .iv path to
 * load an external file. Drag with the left mouse button to rotate. WASD
 * moves the camera. Esc quits.
 *
 * Note: This example uses SDL3, so you do not need any SoGUI toolkit.
 */

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <Inventor/SbRotation.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SoSceneManager.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/nodes/SoCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoSeparator.h>

#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#ifndef COIN_SOURCE_DIR
#define COIN_SOURCE_DIR "."
#endif

void check_error(const bool res)
{
    if (!res) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
    }
}

template <typename T>
T* check_error(T* ptr)
{
    if (!ptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
    }

    return ptr;
}

static const char * profile_name(void)
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

static std::string basename_of(const std::string & path)
{
    const std::string::size_type slash = path.find_last_of("/\\");
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

static bool file_readable(const std::string & path)
{
    std::ifstream in(path.c_str());
    return in.good();
}

static std::string resolve_path(const std::string & path)
{
    if (path.empty()) {
        return path;
    }
    if (file_readable(path)) {
        return path;
    }
    const std::string fromsource = std::string(COIN_SOURCE_DIR) + "/" + path;
    if (file_readable(fromsource)) {
        return fromsource;
    }
    return path;
}

static std::vector<std::string> load_p0_scenes(void)
{
    const std::string manifest = resolve_path("tools/glparity/scenes_p0.txt");
    std::ifstream in(manifest.c_str());
    std::vector<std::string> scenes;
    if (!in) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Could not read P0 manifest: %s", manifest.c_str());
        return scenes;
    }
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.resize(line.size() - 1);
        }
        if (line.empty() || line[0] == '#') {
            continue;
        }
        scenes.push_back(line);
    }
    return scenes;
}

static void apply_rotation(SoRotation * rotation, float rot_x, float rot_y)
{
    SbRotation rx(SbVec3f(1.0f, 0.0f, 0.0f), rot_x);
    SbRotation ry(SbVec3f(0.0f, 1.0f, 0.0f), rot_y);
    rotation->rotation = ry * rx;
}

struct LoadedScene
{
    SoSeparator * harness;
    SoCamera * camera;
    SoRotation * rotation;
    std::string label;
};

static LoadedScene load_scene(const std::string & requested,
                              SoSceneManager * manager)
{
    LoadedScene loaded;
    loaded.harness = NULL;
    loaded.camera = NULL;
    loaded.rotation = NULL;

    const std::string path = resolve_path(requested);
    SoInput input;
    if (!input.openFile(path.c_str())) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Could not open scene: %s", path.c_str());
        return loaded;
    }
    SoSeparator * scene = SoDB::readAll(&input);
    input.closeFile();
    if (!scene) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Could not read scene: %s", path.c_str());
        return loaded;
    }
    scene->ref();

    SoSearchAction search;
    search.setType(SoCamera::getClassTypeId(), TRUE);
    search.setInterest(SoSearchAction::FIRST);
    search.apply(scene);
    SoCamera * camera = search.getPath()
        ? static_cast<SoCamera *>(search.getPath()->getTail())
        : NULL;

    SoSeparator * harness = new SoSeparator;
    harness->ref();
    SoPerspectiveCamera * inserted = NULL;
    if (!camera) {
        inserted = new SoPerspectiveCamera;
        camera = inserted;
        harness->addChild(camera);
    }

    SoDirectionalLight * light = new SoDirectionalLight;
    light->direction = SbVec3f(0.35f, -1.0f, -0.6f);
    light->intensity = 1.0f;
    harness->addChild(light);

    SoRotation * rotation = new SoRotation;
    rotation->rotation = SbRotation::identity();
    harness->addChild(rotation);
    harness->addChild(scene);
    scene->unref();

    manager->setSceneGraph(harness);
    if (inserted) {
        camera->viewAll(harness, manager->getViewportRegion());
        const float dist = std::fabs(camera->position.getValue()[2]);
        camera->nearDistance = dist * 0.1f;
        camera->farDistance = dist * 10.0f;
    }

    loaded.harness = harness;
    loaded.camera = camera;
    loaded.rotation = rotation;
    loaded.label = basename_of(path);
    return loaded;
}

static void update_title(SDL_Window * window, const std::string & label)
{
    const std::string title = std::string("Coin3D SDL3 | ") + profile_name()
        + " | " + label;
    SDL_SetWindowTitle(window, title.c_str());
}

static void log_help(void)
{
    SDL_Log("Controls:");
    SDL_Log("  [ ] or Left/Right  previous / next P0 scene");
    SDL_Log("  1-9, 0             jump to P0 scene 1-10");
    SDL_Log("  R                  return to built-in P0 list");
    SDL_Log("  Space              toggle auto-rotate (off by default)");
    SDL_Log("  Left-drag          rotate scene");
    SDL_Log("  WASD               move camera");
    SDL_Log("  Esc                quit");
    SDL_Log("  sdl3-example file.iv   load an external Inventor file");
}

int main(int argc, char ** argv)
{
    check_error(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

#if defined(COIN_GLES3)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(COIN_GLES2)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(COIN_GL3_CORE)
    /* macOS only exposes Core ≥3.2 when forward-compatible is requested. */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    SDL_Window* window = check_error(SDL_CreateWindow(
        "Coin3D SDL3", 800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY));
    if (!window) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        window = check_error(SDL_CreateWindow(
            "Coin3D SDL3", 800, 600,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY));
    }
    if (!window) {
        return 1;
    }

    SDL_GLContext glctx = check_error(SDL_GL_CreateContext(window));
    if (!glctx) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    check_error(SDL_GL_MakeCurrent(window, glctx));
    check_error(SDL_GL_SetSwapInterval(1));

    {
        const GLubyte * ver = glGetString(GL_VERSION);
        const GLubyte * renderer = glGetString(GL_RENDERER);
        SDL_Log("OpenGL version: %s", ver ? (const char *)ver : "(null)");
        SDL_Log("OpenGL renderer: %s", renderer ? (const char *)renderer : "(null)");
        SDL_Log("Coin profile: %s", profile_name());
#if defined(COIN_GL3_CORE)
        int major = 0, minor = 0;
        if (ver) {
          sscanf((const char *)ver, "%d.%d", &major, &minor);
        }
        if (major < 3 || (major == 3 && minor < 2)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Need OpenGL 3.2 Core for COIN_GL_PROFILE=GL3, got %d.%d (%s)",
                         major, minor, ver ? (const char *)ver : "?");
            SDL_GL_DestroyContext(glctx);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
#endif
        while (glGetError() != GL_NO_ERROR) { }
    }

    glEnable(GL_DEPTH_TEST);
    {
        int samples = 0;
        SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &samples);
        if (samples > 1) {
            glEnable(GL_MULTISAMPLE);
        }
        SDL_Log("MSAA samples: %d", samples);
    }
#ifndef COIN_GL_MODERN
    glEnable(GL_LIGHTING);
#endif

    SoDB::init();
    log_help();

    const std::vector<std::string> p0 = load_p0_scenes();
    if (p0.empty() && argc < 2) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No built-in P0 scenes available.");
        SDL_GL_DestroyContext(glctx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SoSceneManager * scene_manager = new SoSceneManager;
    scene_manager->setBackgroundColor(SbColor(0.12f, 0.14f, 0.18f));
    scene_manager->setWindowSize({800, 600});
    scene_manager->activate();

    {
        int pw = 0;
        int ph = 0;
        SDL_GetWindowSizeInPixels(window, &pw, &ph);
        if (pw > 0 && ph > 0) {
            scene_manager->setWindowSize(SbVec2s((short)pw, (short)ph));
            SDL_Log("drawable size: %dx%d", pw, ph);
        }
    }

    int builtin_index = 0;
    std::string initial = (!p0.empty()) ? p0[0] : "";
    if (argc >= 2 && argv[1] && argv[1][0]) {
        initial = argv[1];
    }

    LoadedScene current = load_scene(initial, scene_manager);
    if (!current.harness) {
        delete scene_manager;
        SDL_GL_DestroyContext(glctx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    update_title(window, current.label);

    bool running = true;
    bool dragging = false;
    bool auto_rotate = false;
    float last_mouse_x = 0.0f;
    float last_mouse_y = 0.0f;
    float rot_y = 0.0f;
    float rot_x = 0.0f;
    Uint64 last_ticks = SDL_GetTicks();

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_WINDOW_RESIZED: {
                    int w = 0;
                    int h = 0;
                    SDL_GetWindowSizeInPixels(window, &w, &h);
                    if (w > 0 && h > 0) {
                        scene_manager->setWindowSize(SbVec2s((short)w, (short)h));
                    }
                    break;
                }
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        dragging = true;
                        last_mouse_x = event.button.x;
                        last_mouse_y = event.button.y;
                    }
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        dragging = false;
                    }
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    if (dragging) {
                        const float dx = event.motion.x - last_mouse_x;
                        const float dy = event.motion.y - last_mouse_y;
                        last_mouse_x = event.motion.x;
                        last_mouse_y = event.motion.y;
                        rot_y += dx * 0.01f;
                        rot_x += dy * 0.01f;
                    }
                    break;
                case SDL_EVENT_KEY_DOWN: {
                    if (event.key.repeat) {
                        break;
                    }
                    const SDL_Scancode code = event.key.scancode;
                    int next_index = -1;
                    bool load_builtin = false;

                    if (code == SDL_SCANCODE_ESCAPE) {
                        running = false;
                    }
                    else if (code == SDL_SCANCODE_SPACE) {
                        auto_rotate = !auto_rotate;
                        SDL_Log("auto-rotate: %s", auto_rotate ? "on" : "off");
                    }
                    else if (code == SDL_SCANCODE_R) {
                        if (!p0.empty()) {
                            if (builtin_index < 0 ||
                                builtin_index >= static_cast<int>(p0.size())) {
                                builtin_index = 0;
                            }
                            load_builtin = true;
                            next_index = builtin_index;
                        }
                    }
                    else if (code == SDL_SCANCODE_LEFTBRACKET ||
                             code == SDL_SCANCODE_LEFT) {
                        if (!p0.empty()) {
                            builtin_index = (builtin_index - 1 +
                                             static_cast<int>(p0.size())) %
                                            static_cast<int>(p0.size());
                            load_builtin = true;
                            next_index = builtin_index;
                        }
                    }
                    else if (code == SDL_SCANCODE_RIGHTBRACKET ||
                             code == SDL_SCANCODE_RIGHT) {
                        if (!p0.empty()) {
                            builtin_index = (builtin_index + 1) %
                                            static_cast<int>(p0.size());
                            load_builtin = true;
                            next_index = builtin_index;
                        }
                    }
                    else if (code >= SDL_SCANCODE_1 && code <= SDL_SCANCODE_9) {
                        const int idx = static_cast<int>(code - SDL_SCANCODE_1);
                        if (idx < static_cast<int>(p0.size())) {
                            builtin_index = idx;
                            load_builtin = true;
                            next_index = builtin_index;
                        }
                    }
                    else if (code == SDL_SCANCODE_0) {
                        if (static_cast<int>(p0.size()) >= 10) {
                            builtin_index = 9;
                            load_builtin = true;
                            next_index = builtin_index;
                        }
                    }

                    if (load_builtin && next_index >= 0) {
                        LoadedScene next = load_scene(p0[next_index], scene_manager);
                        if (next.harness) {
                            current.harness->unref();
                            current = next;
                            rot_x = 0.0f;
                            rot_y = 0.0f;
                            update_title(window, current.label);
                            SDL_Log("scene: %s", current.label.c_str());
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }

        auto keys = SDL_GetKeyboardState(nullptr);
        if (current.camera) {
            SbVec3f position = current.camera->position.getValue();
            const float dt_move = 0.04f;
            if (keys[SDL_SCANCODE_W]) {
                current.camera->position.setValue(position + SbVec3f(0, 0, -1) * dt_move);
            }
            if (keys[SDL_SCANCODE_S]) {
                current.camera->position.setValue(position + SbVec3f(0, 0, 1) * dt_move);
            }
            if (keys[SDL_SCANCODE_A]) {
                current.camera->position.setValue(position + SbVec3f(-1, 0, 0) * dt_move);
            }
            if (keys[SDL_SCANCODE_D]) {
                current.camera->position.setValue(position + SbVec3f(1, 0, 0) * dt_move);
            }
        }

        const Uint64 now = SDL_GetTicks();
        const float dt = (now - last_ticks) / 1000.0f;
        last_ticks = now;

        if (auto_rotate && !dragging) {
            rot_y += dt * 0.8f;
        }
        apply_rotation(current.rotation, rot_x, rot_y);

        SoDB::getSensorManager()->processTimerQueue();
        SoDB::getSensorManager()->processDelayQueue(TRUE);

        scene_manager->render();
        check_error(SDL_GL_SwapWindow(window));
    }

    current.harness->unref();
    delete scene_manager;

    SDL_GL_DestroyContext(glctx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
