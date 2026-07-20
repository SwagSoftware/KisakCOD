#include "kisak_android_native_renderer.h"

#include "kisak_android_fs_runtime.h"
#include "kisak_fastfile_android.h"
#include "kisak_iwi_texture_android.h"
#include "kisak_menu_scene_android.h"
#include "kisak_menu_state_android.h"
#include "kisak_world_scene_android.h"

#include <cmath>

#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <android/native_window.h>

namespace {
constexpr const char* kLogTag = "KisakCODAndroid";
uint32_t g_frameId = 0;
std::atomic_bool g_stopRenderLoop{true};
std::mutex g_renderThreadMutex;
std::thread g_renderThread;
std::mutex g_touchMutex;
float g_touchX = 0.0f;
float g_touchY = 0.0f;
int g_touchAction = 3;
uint32_t g_touchSerial = 0;
std::atomic_bool g_menuBackRequested{false};
std::atomic_bool g_menuTextInputChanged{false};

enum TouchAction {
    kTouchDown = 0,
    kTouchMove = 1,
    kTouchUp = 2,
    kTouchCancel = 3,
};

struct MenuDrawRun {
    int textureIndex;
    GLint firstVertex;
    GLsizei vertexCount;
};

struct WorldDrawRun {
    int textureIndex;
    int lightmapIndex;
    uint32_t firstIndex;
    uint32_t indexCount;
    float alphaTestRef;
    bool blended;
    uint8_t srcBlend; // D3DBLEND factors from the material state bits
    uint8_t dstBlend;
    bool cullNone;
    bool sky;
};

GLenum GlBlendFactor(uint8_t d3dBlend, GLenum fallback) {
    switch (d3dBlend) {
        case 1: return GL_ZERO;
        case 2: return GL_ONE;
        case 3: return GL_SRC_COLOR;
        case 4: return GL_ONE_MINUS_SRC_COLOR;
        case 5: return GL_SRC_ALPHA;
        case 6: return GL_ONE_MINUS_SRC_ALPHA;
        case 7: return GL_DST_ALPHA;
        case 8: return GL_ONE_MINUS_DST_ALPHA;
        case 9: return GL_DST_COLOR;
        case 10: return GL_ONE_MINUS_DST_COLOR;
        default: return fallback;
    }
}

// One unique static model: its instance range plus per-surface draw ranges
// into the shared model vertex/index buffers.
struct WorldModelDraw {
    uint32_t firstInstance;
    GLsizei instanceCount;
    bool anyOpaque;
    bool anyBlended;
    std::vector<WorldDrawRun> surfaces;
};

// Background map-zone loader: the launch command kicks a worker thread that
// decompresses the map fastfile, runs the zone loader, and publishes the
// extracted world scene for the render thread to upload.
std::mutex g_worldLoadMutex;
std::shared_ptr<KisakWorldScene> g_pendingWorldScene;
std::atomic_bool g_worldLoadInProgress{false};

struct GlContext {
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext context = EGL_NO_CONTEXT;
    GLuint program = 0;
    GLuint vertexBuffer = 0;
    GLuint assetTexture = 0;
    GLint phaseUniform = -1;
    GLint textureUniform = -1;
    GLint scaleUniform = -1;
    int width = 0;
    int height = 0;
    int texWidth = 1;
    int texHeight = 1;

    // COD4 menu scene rendering.
    bool menuReady = false;
    bool menuInitTried = false;
    GLuint menuProgram = 0;
    GLuint menuVbo = 0;
    GLuint menuCursorVbo = 0;
    GLuint whiteTexture = 0;
    GLint menuScaleUniform = -1;
    GLint menuOffsetUniform = -1;
    GLint menuTextureUniform = -1;
    std::vector<GLuint> menuTextures;
    std::vector<MenuDrawRun> menuRuns;
    std::vector<KisakMenuHitbox> menuHitboxes;
    std::string menuName;
    std::string menuTargetName = "main";
    std::string pendingMenuTargetName;
    std::vector<std::string> menuStack;
    float menuCanvasW = 854.0f;
    float menuCanvasH = 480.0f;
    uint32_t menuZoneGeneration = 0;
    int menuLoggedHitbox = -1;
    int menuPressedHitbox = -1;
    uint32_t menuActivatedTouchSerial = 0;

    // UI script runtime: zone/menu the current scene came from (zones kept
    // alive here), last menu whose onOpen ran, and the rebuild flag set when
    // scripts change dvars/localvars/visibility.
    std::vector<std::shared_ptr<const KisakZoneLoadResult>> menuSceneZones;
    const KisakZoneLoadResult* menuSceneZone = nullptr;
    uint32_t menuSceneMenuRef = 0;
    std::string lastOnOpenMenu;
    bool menuSceneDirty = false;
    bool menuTimeDependent = false;
    bool suppressNextMenuSceneLog = false;
    uint32_t lastMenuTimeRefreshFrame = 0;
    // Decoded IWI textures survive scene rebuilds (focus changes rebuild the
    // scene every time; re-decoding DXT atlases each time would stall).
    std::map<std::string, GLuint> menuTextureCache;

    // World viewer (mission launch): static GfxWorld geometry with a
    // touch-driven free camera.
    bool worldReady = false;
    GLuint worldProgram = 0;
    GLuint worldVbo = 0;
    GLuint worldIbo = 0;
    GLint worldMvpUniform = -1;
    GLint worldTextureUniform = -1;
    GLint worldAlphaRefUniform = -1;
    GLint worldLightmapUniform = -1;
    GLint worldSunMaskUniform = -1;
    GLint worldSunColorUniform = -1;
    std::vector<GLuint> worldTextures;
    std::vector<GLuint> worldLightmapTextures;    // secondary: radiosity color
    std::vector<GLuint> worldSunMaskTextures;     // primary: L8 sun visibility
    GLuint worldGrayTexture = 0;  // neutral color page: lightmap*2 == 1
    GLuint worldBlackTexture = 0; // neutral sun mask: adds nothing
    float worldSunColor[3] = {1.0f, 1.0f, 0.9f};
    // Sky pass (cubemap sampled by world direction).
    GLuint worldSkyProgram = 0;
    GLuint worldSkyTexture = 0;
    GLint worldSkyMvpUniform = -1;
    GLint worldSkyCamUniform = -1;
    GLint worldSkySamplerUniform = -1;
    // Walkable heightfield + collision brushes for the walking camera.
    KisakWorldHeightfield worldGround;
    std::vector<KisakWorldBrush> worldBrushes;
    std::vector<float> worldBrushPlanes;
    std::vector<WorldDrawRun> worldRuns;
    // Static models (GLES3 instancing).
    GLuint worldModelProgram = 0;
    GLuint worldModelVbo = 0;
    GLuint worldModelIbo = 0;
    GLuint worldModelInstanceVbo = 0;
    GLint worldModelMvpUniform = -1;
    GLint worldModelTextureUniform = -1;
    GLint worldModelAlphaRefUniform = -1;
    std::vector<WorldModelDraw> worldModels;
    std::map<std::string, GLuint> worldTextureCache;
    std::string worldName;
    float camPos[3] = {0.0f, 0.0f, 0.0f};
    float camYaw = 0.0f;   // degrees, COD4 convention (0 = +X, Z-up)
    float camPitch = 0.0f; // degrees, positive looks up
    // Touch camera state: one finger, left half = virtual move stick,
    // right half = look.
    int worldTouchMode = 0; // 0 none, 1 move, 2 look
    float worldTouchStartX = 0.0f;
    float worldTouchStartY = 0.0f;
    float worldTouchLastX = 0.0f;
    float worldTouchLastY = 0.0f;
};

struct TouchSnapshot {
    float x = 0.0f;
    float y = 0.0f;
    int action = kTouchCancel;
    uint32_t serial = 0;
};

TouchSnapshot GetTouchSnapshot() {
    std::lock_guard<std::mutex> lock(g_touchMutex);
    return {g_touchX, g_touchY, g_touchAction, g_touchSerial};
}

void ConsumeTouchSnapshot(uint32_t serial) {
    std::lock_guard<std::mutex> lock(g_touchMutex);
    if (g_touchSerial == serial) {
        g_touchAction = kTouchCancel;
    }
}

uint32_t Argb(uint8_t red, uint8_t green, uint8_t blue) {
    return 0xff000000u
        | (static_cast<uint32_t>(red) << 16)
        | (static_cast<uint32_t>(green) << 8)
        | static_cast<uint32_t>(blue);
}

void FillRect(
    uint32_t* pixels,
    int stride,
    int width,
    int height,
    int x,
    int y,
    int rectWidth,
    int rectHeight,
    uint32_t color
) {
    const int left = std::clamp(x, 0, width);
    const int top = std::clamp(y, 0, height);
    const int right = std::clamp(x + rectWidth, 0, width);
    const int bottom = std::clamp(y + rectHeight, 0, height);
    for (int row = top; row < bottom; ++row) {
        uint32_t* line = pixels + row * stride;
        for (int col = left; col < right; ++col) {
            line[col] = color;
        }
    }
}

void DrawVignette(uint32_t* pixels, int stride, int width, int height) {
    const int centerX = width / 2;
    const int centerY = height / 2;
    const int maxDistance = std::max(1, centerX + centerY);
    for (int y = 0; y < height; ++y) {
        uint32_t* line = pixels + y * stride;
        for (int x = 0; x < width; ++x) {
            const int distance = std::abs(x - centerX) + std::abs(y - centerY);
            const int shade = std::clamp(28 - (distance * 22 / maxDistance), 6, 28);
            const int tactical = ((x / 24) ^ (y / 24)) & 1;
            line[x] = Argb(
                static_cast<uint8_t>(shade + tactical * 5),
                static_cast<uint8_t>(shade + 8),
                static_cast<uint8_t>(shade + tactical * 3)
            );
        }
    }
}

void DrawDiagnosticBars(uint32_t* pixels, int stride, int width, int height, bool fsReady, uint32_t frameId) {
    const uint32_t amber = Argb(217, 177, 95);
    const uint32_t olive = Argb(118, 143, 86);
    const uint32_t red = Argb(173, 68, 57);
    const uint32_t muted = Argb(51, 61, 44);
    const uint32_t bright = fsReady ? olive : red;

    const int bandTop = height / 2 - 54;
    FillRect(pixels, stride, width, height, width / 8, bandTop, width * 3 / 4, 10, amber);
    FillRect(pixels, stride, width, height, width / 8, bandTop + 24, width * 3 / 4, 18, muted);
    FillRect(pixels, stride, width, height, width / 8, bandTop + 24, fsReady ? width * 3 / 4 : width / 4, 18, bright);
    FillRect(pixels, stride, width, height, width / 8, bandTop + 58, width * 3 / 4, 10, amber);

    const int markerCount = 8;
    const int markerSize = std::max(8, width / 80);
    const int markerGap = markerSize + 8;
    const int startX = width / 2 - (markerCount * markerGap) / 2;
    const int y = bandTop + 88;
    for (int i = 0; i < markerCount; ++i) {
        const bool active = fsReady && (i <= static_cast<int>(frameId % markerCount));
        FillRect(
            pixels,
            stride,
            width,
            height,
            startX + i * markerGap,
            y,
            markerSize,
            markerSize,
            active ? amber : muted
        );
    }
}

bool InitEgl(ANativeWindow* window, GlContext& gl) {
    gl.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (gl.display == EGL_NO_DISPLAY) {
        return false;
    }
    if (eglInitialize(gl.display, nullptr, nullptr) != EGL_TRUE) {
        return false;
    }

    constexpr EGLint configAttributes[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24, // the world pass depth-tests
        EGL_STENCIL_SIZE, 0,
        EGL_NONE
    };

    EGLConfig config = nullptr;
    EGLint configCount = 0;
    if (eglChooseConfig(gl.display, configAttributes, &config, 1, &configCount) != EGL_TRUE || configCount == 0) {
        // Retry with a 16-bit depth buffer before giving up.
        EGLint fallbackAttributes[std::size(configAttributes)];
        std::copy(std::begin(configAttributes), std::end(configAttributes), fallbackAttributes);
        for (size_t i = 0; i + 1 < std::size(fallbackAttributes); i += 2) {
            if (fallbackAttributes[i] == EGL_DEPTH_SIZE) {
                fallbackAttributes[i + 1] = 16;
            }
        }
        if (eglChooseConfig(gl.display, fallbackAttributes, &config, 1, &configCount) != EGL_TRUE
            || configCount == 0) {
            return false;
        }
    }

    EGLint nativeVisualId = 0;
    eglGetConfigAttrib(gl.display, config, EGL_NATIVE_VISUAL_ID, &nativeVisualId);
    ANativeWindow_setBuffersGeometry(window, 0, 0, nativeVisualId);

    gl.surface = eglCreateWindowSurface(gl.display, config, window, nullptr);
    if (gl.surface == EGL_NO_SURFACE) {
        return false;
    }

    constexpr EGLint contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    gl.context = eglCreateContext(gl.display, config, EGL_NO_CONTEXT, contextAttributes);
    if (gl.context == EGL_NO_CONTEXT) {
        return false;
    }
    if (eglMakeCurrent(gl.display, gl.surface, gl.surface, gl.context) != EGL_TRUE) {
        return false;
    }

    eglSwapInterval(gl.display, 1);
    eglQuerySurface(gl.display, gl.surface, EGL_WIDTH, &gl.width);
    eglQuerySurface(gl.display, gl.surface, EGL_HEIGHT, &gl.height);

    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    __android_log_print(
        ANDROID_LOG_INFO,
        kLogTag,
        "EGL renderer ready: %dx%d vendor=%s renderer=%s version=%s",
        gl.width,
        gl.height,
        vendor ? vendor : "<unknown>",
        renderer ? renderer : "<unknown>",
        version ? version : "<unknown>"
    );
    return true;
}

GLuint CompileShader(GLenum type, const char* source) {
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        char log[512]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "Shader compile failed: %s", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint CreateTexturedQuadProgram() {
    constexpr const char* kVertexShader = R"(#version 300 es
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
uniform float uPhase;
uniform vec2 uScale;
out vec2 vTexCoord;
void main() {
    float breathe = 1.0 + sin(uPhase * 6.2831853) * 0.02;
    gl_Position = vec4(aPosition * uScale * breathe, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
)";

    constexpr const char* kFragmentShader = R"(#version 300 es
precision mediump float;
in vec2 vTexCoord;
uniform sampler2D uTexture;
out vec4 fragColor;
void main() {
    fragColor = texture(uTexture, vTexCoord);
}
)";

    const GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, kVertexShader);
    if (vertexShader == 0) {
        return 0;
    }
    const GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, kFragmentShader);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return 0;
    }

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        char log[512]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "Program link failed: %s", log);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

GLuint UploadRgbaTexture(const uint8_t* pixels, int width, int height) {
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    return texture;
}

// Loads the first decodable IWI from a list of well-known COD4 textures and
// uploads its full-resolution mip as an RGBA texture.
GLuint CreateAssetTexture(int& texWidth, int& texHeight) {
    constexpr uint32_t kMaxIwiBytes = 16 * 1024 * 1024;
    constexpr const char* kCandidates[] = {
        "images/compassface.iwi",
        "images/ac130_hud_target.iwi",
        "images/$white.iwi",
    };

    for (const char* qpath : kCandidates) {
        const KisakVirtualFileRead iwi = AndroidFsReadFile(qpath, kMaxIwiBytes);
        if (!iwi.found || !iwi.error.empty() || iwi.data.empty()) {
            __android_log_print(
                ANDROID_LOG_WARN,
                kLogTag,
                "Renderer IWI read failed: %s",
                DescribeVirtualFileRead(iwi).c_str()
            );
            continue;
        }

        const KisakIwiTexture decoded = DecodeIwiTexture(iwi.data);
        if (!decoded.valid) {
            __android_log_print(
                ANDROID_LOG_WARN,
                kLogTag,
                "Renderer IWI decode failed: %s %s",
                qpath,
                DescribeIwiTexture(decoded).c_str()
            );
            continue;
        }

        const GLuint texture = UploadRgbaTexture(decoded.rgba.data(), decoded.width, decoded.height);
        texWidth = decoded.width;
        texHeight = decoded.height;
        __android_log_print(
            ANDROID_LOG_INFO,
            kLogTag,
            "Renderer IWI texture ready: texture=%u %s (%s depuis %s)",
            texture,
            DescribeIwiTexture(decoded).c_str(),
            qpath,
            iwi.source.c_str()
        );
        return texture;
    }

    const uint8_t fallback[] = {
        217, 177, 95, 255, 51, 61, 44, 255,
        51, 61, 44, 255, 217, 177, 95, 255,
    };
    texWidth = 2;
    texHeight = 2;
    const GLuint texture = UploadRgbaTexture(fallback, 2, 2);
    __android_log_print(
        ANDROID_LOG_WARN,
        kLogTag,
        "Renderer IWI texture fallback: aucune texture decodable, damier 2x2 texture=%u",
        texture
    );
    return texture;
}

GLuint CreateMenuProgram() {
    constexpr const char* kVertexShader = R"(#version 300 es
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;
uniform vec2 uScale;
uniform vec2 uOffset;
out vec2 vTexCoord;
out vec4 vColor;
void main() {
    gl_Position = vec4(aPosition * uScale + uOffset, 0.0, 1.0);
    vTexCoord = aTexCoord;
    vColor = aColor;
}
)";

    constexpr const char* kFragmentShader = R"(#version 300 es
precision mediump float;
in vec2 vTexCoord;
in vec4 vColor;
uniform sampler2D uTexture;
out vec4 fragColor;
void main() {
    fragColor = texture(uTexture, vTexCoord) * vColor;
}
)";

    const GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, kVertexShader);
    const GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, kFragmentShader);
    if (vertexShader == 0 || fragmentShader == 0) {
        return 0;
    }
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        char log[512]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "Menu program link failed: %s", log);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

// Returns 0 for missing/undecodable images (e.g. wavelet-compressed) so the
// draw pass can skip them instead of showing a white placeholder.
GLuint CreateMenuTexture(GlContext& gl, const std::string& imageName) {
    const auto cached = gl.menuTextureCache.find(imageName);
    if (cached != gl.menuTextureCache.end()) {
        return cached->second;
    }
    const KisakVirtualFileRead iwi = AndroidFsReadFile("images/" + imageName + ".iwi", 16 * 1024 * 1024);
    if (!iwi.found || !iwi.error.empty() || iwi.data.empty()) {
        __android_log_print(
            ANDROID_LOG_WARN,
            kLogTag,
            "Menu texture '%s' introuvable, quad ignore",
            imageName.c_str()
        );
        gl.menuTextureCache[imageName] = 0;
        return 0;
    }
    const KisakIwiTexture decoded = DecodeIwiTexture(iwi.data);
    if (!decoded.valid) {
        __android_log_print(
            ANDROID_LOG_WARN,
            kLogTag,
            "Menu texture '%s': %s",
            imageName.c_str(),
            DescribeIwiTexture(decoded).c_str()
        );
        gl.menuTextureCache[imageName] = 0;
        return 0;
    }
    __android_log_print(
        ANDROID_LOG_INFO,
        kLogTag,
        "Menu texture '%s' %s",
        imageName.c_str(),
        DescribeIwiTexture(decoded).c_str()
    );
    const GLuint texture = UploadRgbaTexture(decoded.rgba.data(), decoded.width, decoded.height);
    gl.menuTextureCache[imageName] = texture;
    return texture;
}

void DestroyMenuSceneResources(GlContext& gl) {
    if (gl.menuVbo != 0) {
        glDeleteBuffers(1, &gl.menuVbo);
        gl.menuVbo = 0;
    }
    if (gl.menuCursorVbo != 0) {
        glDeleteBuffers(1, &gl.menuCursorVbo);
        gl.menuCursorVbo = 0;
    }
    // Decoded textures stay in menuTextureCache across rebuilds; they die
    // with the EGL context.
    gl.menuTextures.clear();
    if (gl.whiteTexture != 0) {
        glDeleteTextures(1, &gl.whiteTexture);
        gl.whiteTexture = 0;
    }
    if (gl.menuProgram != 0) {
        glDeleteProgram(gl.menuProgram);
        gl.menuProgram = 0;
    }
    gl.menuRuns.clear();
    gl.menuHitboxes.clear();
    gl.menuName.clear();
    gl.menuReady = false;
    gl.menuInitTried = false;
    gl.menuTimeDependent = false;
    gl.menuScaleUniform = -1;
    gl.menuOffsetUniform = -1;
    gl.menuTextureUniform = -1;
    gl.menuLoggedHitbox = -1;
    gl.menuPressedHitbox = -1;
    gl.menuActivatedTouchSerial = 0;
}

void QueueMenuNavigation(GlContext& gl, const std::string& target, bool pushCurrent) {
    if (target.empty()) {
        return;
    }
    if (pushCurrent && !gl.menuName.empty() && gl.menuName != target) {
        gl.menuStack.push_back(gl.menuName);
    }
    // onOpen is per menu activation, not a once-per-name initializer. Going
    // back to main_text after Options must rerun its focus/show/hide scripts.
    gl.lastOnOpenMenu.clear();
    ClearKisakMenuFocus();
    gl.pendingMenuTargetName = target;
}

void QueueMenuBack(GlContext& gl) {
    std::string target = "main";
    if (!gl.menuStack.empty()) {
        target = gl.menuStack.back();
        gl.menuStack.pop_back();
    }
    QueueMenuNavigation(gl, target, false);
}

std::string LaunchCommandMapName(const std::string& command) {
    std::istringstream in(command);
    std::string verb;
    std::string mapName;
    in >> verb >> mapName;
    return mapName;
}

// Decompresses and deserializes the map zone off the render thread, then
// publishes the extracted world scene.
void StartWorldLoad(const std::string& mapName) {
    if (mapName.empty() || !AndroidFsInitialized()) {
        return;
    }
    bool expected = false;
    if (!g_worldLoadInProgress.compare_exchange_strong(expected, true)) {
        __android_log_print(ANDROID_LOG_WARN, kLogTag, "Chargement de monde deja en cours");
        return;
    }
    const std::string zonePath = AndroidFsBasePath() + "/zone/english/" + mapName + ".ff";
    __android_log_print(ANDROID_LOG_INFO, kLogTag, "Chargement du monde: %s", zonePath.c_str());
    std::thread([zonePath, mapName]() {
        auto scene = std::make_shared<KisakWorldScene>();
        std::vector<uint8_t> zoneData;
        const KisakFastFileProbe probe = ProbeFastFile(zonePath, &zoneData, 1024ull << 20);
        if (!probe.fullyDecompressed) {
            scene->error = "decompression de " + mapName + ".ff: " + probe.error;
        } else {
            const KisakZoneLoadResult zone = LoadZoneAssets(zoneData);
            zoneData.clear();
            zoneData.shrink_to_fit();
            __android_log_print(
                ANDROID_LOG_INFO,
                kLogTag,
                "Zone de mission '%s': %s",
                mapName.c_str(),
                DescribeZoneLoad(zone).c_str()
            );
            if (!zone.error.empty()) {
                scene->error = "zone " + mapName + ": " + zone.error;
            } else {
                *scene = BuildWorldScene(zone);
                // Pre-decode the IWI colormaps here: decoding ~400 DXT
                // textures on the render thread stalls frame submission for
                // minutes and gets the app hidden by the system.
                scene->decodedTextures.resize(scene->textureImages.size());
                for (size_t i = 0; i < scene->textureImages.size(); ++i) {
                    const KisakVirtualFileRead iwi = AndroidFsReadFile(
                        "images/" + scene->textureImages[i] + ".iwi", 16 * 1024 * 1024);
                    if (iwi.found && iwi.error.empty() && !iwi.data.empty()) {
                        scene->decodedTextures[i] = DecodeIwiTexture(iwi.data);
                    }
                }
                if (!scene->skyImage.empty()) {
                    const KisakVirtualFileRead iwi = AndroidFsReadFile(
                        "images/" + scene->skyImage + ".iwi", 32 * 1024 * 1024);
                    if (iwi.found && iwi.error.empty() && !iwi.data.empty()) {
                        scene->skyCubemap = DecodeIwiCubemap(iwi.data);
                    }
                    __android_log_print(
                        ANDROID_LOG_INFO,
                        kLogTag,
                        "Ciel '%s': %s",
                        scene->skyImage.c_str(),
                        scene->skyCubemap.valid ? "cubemap decode" : scene->skyCubemap.error.c_str()
                    );
                }
            }
        }
        __android_log_print(
            scene->error.empty() ? ANDROID_LOG_INFO : ANDROID_LOG_WARN,
            kLogTag,
            "Scene monde '%s': %s",
            mapName.c_str(),
            DescribeWorldScene(*scene).c_str()
        );
        {
            std::lock_guard<std::mutex> lock(g_worldLoadMutex);
            g_pendingWorldScene = std::move(scene);
        }
        g_worldLoadInProgress.store(false);
    }).detach();
}

GLuint CreateWorldProgram() {
    constexpr const char* kVertexShader = R"(#version 300 es
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;
layout(location = 3) in vec2 aLmapCoord;
uniform mat4 uMvp;
out vec2 vTexCoord;
out vec4 vColor;
out vec2 vLmapCoord;
void main() {
    gl_Position = uMvp * vec4(aPosition, 1.0);
    vTexCoord = aTexCoord;
    vColor = aColor;
    vLmapCoord = aLmapCoord;
}
)";
    constexpr const char* kFragmentShader = R"(#version 300 es
precision mediump float;
in vec2 vTexCoord;
in vec4 vColor;
in vec2 vLmapCoord;
uniform sampler2D uTexture;
uniform sampler2D uLightmap;
uniform sampler2D uSunMask;
uniform vec3 uSunColor;
uniform float uAlphaRef;
out vec4 fragColor;
void main() {
    // IW3 lightmap model: the secondary page is baked radiosity color (2x
    // overbright) and the primary page an L8 sun-visibility mask that gates
    // the direct sun term. Unlit runs bind neutral gray/black pages. Vertex
    // color is a layer-blend weight on world surfaces and texture alpha a
    // blend mask on terrain, so neither drives shading directly.
    vec4 texel = texture(uTexture, vTexCoord);
    if (uAlphaRef >= 0.0 && texel.a < uAlphaRef) {
        discard;
    }
    vec3 light = texture(uLightmap, vLmapCoord).rgb * 2.0
        + uSunColor * texture(uSunMask, vLmapCoord).r;
    fragColor = vec4(texel.rgb * min(light, vec3(1.6)), texel.a);
}
)";
    const GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, kVertexShader);
    const GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, kFragmentShader);
    if (vertexShader == 0 || fragmentShader == 0) {
        return 0;
    }
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        char log[512]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "World program link failed: %s", log);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

// Instanced variant for static models: the per-instance scaled basis rows +
// origin come in as divisor-1 attributes.
GLuint CreateWorldModelProgram() {
    constexpr const char* kVertexShader = R"(#version 300 es
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;
layout(location = 3) in vec3 aBasisX;
layout(location = 4) in vec3 aBasisY;
layout(location = 5) in vec3 aBasisZ;
layout(location = 6) in vec3 aOrigin;
uniform mat4 uMvp;
out vec2 vTexCoord;
out vec4 vColor;
void main() {
    vec3 world = aOrigin
        + aPosition.x * aBasisX
        + aPosition.y * aBasisY
        + aPosition.z * aBasisZ;
    gl_Position = uMvp * vec4(world, 1.0);
    vTexCoord = aTexCoord;
    vColor = aColor;
}
)";
    constexpr const char* kFragmentShader = R"(#version 300 es
precision mediump float;
in vec2 vTexCoord;
in vec4 vColor;
uniform sampler2D uTexture;
uniform float uAlphaRef;
out vec4 fragColor;
void main() {
    vec4 texel = texture(uTexture, vTexCoord);
    if (uAlphaRef >= 0.0 && texel.a < uAlphaRef) {
        discard;
    }
    fragColor = vec4(texel.rgb, texel.a);
}
)";
    const GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, kVertexShader);
    const GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, kFragmentShader);
    if (vertexShader == 0 || fragmentShader == 0) {
        return 0;
    }
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        char log[512]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "World model program link failed: %s", log);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

// Sky pass: the sky surfaces' geometry is sampled by world-space direction
// from the camera into the decoded cubemap.
GLuint CreateWorldSkyProgram() {
    constexpr const char* kVertexShader = R"(#version 300 es
layout(location = 0) in vec3 aPosition;
uniform mat4 uMvp;
uniform vec3 uCamPos;
out vec3 vDirection;
void main() {
    vDirection = aPosition - uCamPos;
    vec4 clip = uMvp * vec4(aPosition, 1.0);
    gl_Position = clip.xyww; // pin to the far plane
}
)";
    constexpr const char* kFragmentShader = R"(#version 300 es
precision mediump float;
in vec3 vDirection;
uniform samplerCube uSky;
out vec4 fragColor;
void main() {
    // The cubemap faces are laid out on WORLD axes (+X,-X,+Y,-Y,+Z=up,-Z),
    // and D3D/GL share the same per-face UV convention, so the raw world
    // direction addresses it directly.
    fragColor = vec4(texture(uSky, normalize(vDirection)).rgb, 1.0);
}
)";
    const GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, kVertexShader);
    const GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, kFragmentShader);
    if (vertexShader == 0 || fragmentShader == 0) {
        return 0;
    }
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        char log[512]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "Sky program link failed: %s", log);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

// World textures tile (raw texCoords go well past 1.0), so they need REPEAT
// wrapping and mips, unlike the clamped UI textures.
GLuint CreateWorldTexture(GlContext& gl, const std::string& imageName) {
    const auto cached = gl.worldTextureCache.find(imageName);
    if (cached != gl.worldTextureCache.end()) {
        return cached->second;
    }
    GLuint texture = 0;
    const KisakVirtualFileRead iwi = AndroidFsReadFile("images/" + imageName + ".iwi", 16 * 1024 * 1024);
    if (iwi.found && iwi.error.empty() && !iwi.data.empty()) {
        const KisakIwiTexture decoded = DecodeIwiTexture(iwi.data);
        if (decoded.valid) {
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA, decoded.width, decoded.height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, decoded.rgba.data()
            );
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }
    gl.worldTextureCache[imageName] = texture;
    return texture;
}

void DestroyWorldSceneResources(GlContext& gl) {
    if (gl.worldVbo != 0) {
        glDeleteBuffers(1, &gl.worldVbo);
        gl.worldVbo = 0;
    }
    if (gl.worldIbo != 0) {
        glDeleteBuffers(1, &gl.worldIbo);
        gl.worldIbo = 0;
    }
    if (gl.worldProgram != 0) {
        glDeleteProgram(gl.worldProgram);
        gl.worldProgram = 0;
    }
    if (gl.worldModelVbo != 0) {
        glDeleteBuffers(1, &gl.worldModelVbo);
        gl.worldModelVbo = 0;
    }
    if (gl.worldModelIbo != 0) {
        glDeleteBuffers(1, &gl.worldModelIbo);
        gl.worldModelIbo = 0;
    }
    if (gl.worldModelInstanceVbo != 0) {
        glDeleteBuffers(1, &gl.worldModelInstanceVbo);
        gl.worldModelInstanceVbo = 0;
    }
    if (gl.worldModelProgram != 0) {
        glDeleteProgram(gl.worldModelProgram);
        gl.worldModelProgram = 0;
    }
    gl.worldModels.clear();
    for (auto& [name, texture] : gl.worldTextureCache) {
        if (texture != 0) {
            glDeleteTextures(1, &texture);
        }
    }
    gl.worldTextureCache.clear();
    gl.worldTextures.clear();
    for (GLuint texture : gl.worldLightmapTextures) {
        if (texture != 0) {
            glDeleteTextures(1, &texture);
        }
    }
    gl.worldLightmapTextures.clear();
    for (GLuint texture : gl.worldSunMaskTextures) {
        if (texture != 0) {
            glDeleteTextures(1, &texture);
        }
    }
    gl.worldSunMaskTextures.clear();
    if (gl.worldGrayTexture != 0) {
        glDeleteTextures(1, &gl.worldGrayTexture);
        gl.worldGrayTexture = 0;
    }
    if (gl.worldBlackTexture != 0) {
        glDeleteTextures(1, &gl.worldBlackTexture);
        gl.worldBlackTexture = 0;
    }
    if (gl.worldSkyTexture != 0) {
        glDeleteTextures(1, &gl.worldSkyTexture);
        gl.worldSkyTexture = 0;
    }
    if (gl.worldSkyProgram != 0) {
        glDeleteProgram(gl.worldSkyProgram);
        gl.worldSkyProgram = 0;
    }
    gl.worldGround = {};
    gl.worldBrushes.clear();
    gl.worldBrushPlanes.clear();
    gl.worldRuns.clear();
    gl.worldReady = false;
    gl.worldTouchMode = 0;
}

bool InitWorldScene(GlContext& gl, const KisakWorldScene& scene) {
    if (!scene.error.empty() || scene.vertices.empty() || scene.indices.empty()) {
        return false;
    }
    DestroyWorldSceneResources(gl);
    gl.worldProgram = CreateWorldProgram();
    if (gl.worldProgram == 0) {
        return false;
    }
    gl.worldMvpUniform = glGetUniformLocation(gl.worldProgram, "uMvp");
    gl.worldTextureUniform = glGetUniformLocation(gl.worldProgram, "uTexture");
    gl.worldAlphaRefUniform = glGetUniformLocation(gl.worldProgram, "uAlphaRef");
    gl.worldLightmapUniform = glGetUniformLocation(gl.worldProgram, "uLightmap");
    gl.worldSunMaskUniform = glGetUniformLocation(gl.worldProgram, "uSunMask");
    gl.worldSunColorUniform = glGetUniformLocation(gl.worldProgram, "uSunColor");
    gl.worldSunColor[0] = scene.sunColor[0];
    gl.worldSunColor[1] = scene.sunColor[1];
    gl.worldSunColor[2] = scene.sunColor[2];

    glGenBuffers(1, &gl.worldVbo);
    glBindBuffer(GL_ARRAY_BUFFER, gl.worldVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(scene.vertices.size() * sizeof(float)),
        scene.vertices.data(),
        GL_STATIC_DRAW
    );
    glGenBuffers(1, &gl.worldIbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl.worldIbo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(scene.indices.size() * sizeof(uint32_t)),
        scene.indices.data(),
        GL_STATIC_DRAW
    );

    gl.worldTextures.clear();
    uint32_t missingTextures = 0;
    for (size_t i = 0; i < scene.textureImages.size(); ++i) {
        GLuint texture = 0;
        if (i < scene.decodedTextures.size() && scene.decodedTextures[i].valid) {
            const KisakIwiTexture& decoded = scene.decodedTextures[i];
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA, decoded.width, decoded.height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, decoded.rgba.data()
            );
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            gl.worldTextureCache[scene.textureImages[i]] = texture;
        } else {
            texture = CreateWorldTexture(gl, scene.textureImages[i]);
        }
        if (texture == 0) {
            ++missingTextures;
            __android_log_print(
                ANDROID_LOG_WARN,
                kLogTag,
                "Texture monde manquante: %s",
                scene.textureImages[i].c_str()
            );
        }
        gl.worldTextures.push_back(texture);
    }
    if (missingTextures != 0) {
        __android_log_print(
            ANDROID_LOG_WARN,
            kLogTag,
            "Textures monde manquantes: %u/%zu",
            missingTextures,
            scene.textureImages.size()
        );
    }

    // Lightmap pairs (decoded RGBA from the zone) + neutral pages for unlit
    // runs: gray keeps the 2x radiosity term at identity, black cancels the
    // sun term.
    const auto uploadLightmapPage = [](const KisakIwiTexture& page) -> GLuint {
        if (!page.valid || page.rgba.empty()) {
            return 0;
        }
        GLuint texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, page.width, page.height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, page.rgba.data()
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        return texture;
    };
    const uint8_t neutralGray[4] = {128, 128, 128, 255};
    const uint8_t neutralBlack[4] = {0, 0, 0, 255};
    gl.worldGrayTexture = UploadRgbaTexture(neutralGray, 1, 1);
    gl.worldBlackTexture = UploadRgbaTexture(neutralBlack, 1, 1);
    gl.worldLightmapTextures.clear();
    gl.worldSunMaskTextures.clear();
    for (const KisakWorldLightmap& page : scene.lightmaps) {
        gl.worldLightmapTextures.push_back(uploadLightmapPage(page.secondary));
        gl.worldSunMaskTextures.push_back(uploadLightmapPage(page.primary));
    }

    // Surfaces are sorted (opaque first, then by texture and lightmap) and
    // appended in order, so contiguous same-state surfaces merge into single
    // draw calls.
    gl.worldRuns.clear();
    for (const KisakWorldDrawSurface& surface : scene.surfaces) {
        if (!gl.worldRuns.empty()
            && gl.worldRuns.back().textureIndex == surface.textureIndex
            && gl.worldRuns.back().lightmapIndex == surface.lightmapIndex
            && gl.worldRuns.back().alphaTestRef == surface.alphaTestRef
            && gl.worldRuns.back().blended == surface.blended
            && gl.worldRuns.back().srcBlend == surface.srcBlend
            && gl.worldRuns.back().dstBlend == surface.dstBlend
            && gl.worldRuns.back().cullNone == surface.cullNone
            && gl.worldRuns.back().sky == surface.sky
            && gl.worldRuns.back().firstIndex + gl.worldRuns.back().indexCount == surface.firstIndex) {
            gl.worldRuns.back().indexCount += surface.indexCount;
        } else {
            gl.worldRuns.push_back({
                surface.textureIndex, surface.lightmapIndex,
                surface.firstIndex, surface.indexCount,
                surface.alphaTestRef, surface.blended,
                surface.srcBlend, surface.dstBlend, surface.cullNone,
                surface.sky,
            });
        }
    }

    // Static models: shared mesh pool + per-instance transforms, drawn with
    // glDrawElementsInstanced.
    if (!scene.modelVertices.empty() && !scene.models.empty()) {
        gl.worldModelProgram = CreateWorldModelProgram();
        if (gl.worldModelProgram != 0) {
            gl.worldModelMvpUniform = glGetUniformLocation(gl.worldModelProgram, "uMvp");
            gl.worldModelTextureUniform = glGetUniformLocation(gl.worldModelProgram, "uTexture");
            gl.worldModelAlphaRefUniform = glGetUniformLocation(gl.worldModelProgram, "uAlphaRef");
            glGenBuffers(1, &gl.worldModelVbo);
            glBindBuffer(GL_ARRAY_BUFFER, gl.worldModelVbo);
            glBufferData(
                GL_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(scene.modelVertices.size() * sizeof(float)),
                scene.modelVertices.data(),
                GL_STATIC_DRAW
            );
            glGenBuffers(1, &gl.worldModelIbo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl.worldModelIbo);
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(scene.modelIndices.size() * sizeof(uint32_t)),
                scene.modelIndices.data(),
                GL_STATIC_DRAW
            );
            glGenBuffers(1, &gl.worldModelInstanceVbo);
            glBindBuffer(GL_ARRAY_BUFFER, gl.worldModelInstanceVbo);
            glBufferData(
                GL_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(scene.modelInstances.size() * sizeof(float)),
                scene.modelInstances.data(),
                GL_STATIC_DRAW
            );
            gl.worldModels.clear();
            for (const KisakWorldModelMesh& mesh : scene.models) {
                WorldModelDraw draw;
                draw.firstInstance = mesh.firstInstance;
                draw.instanceCount = static_cast<GLsizei>(mesh.instanceCount);
                draw.anyOpaque = false;
                draw.anyBlended = false;
                for (const KisakWorldDrawSurface& surface : mesh.surfaces) {
                    draw.surfaces.push_back({
                        surface.textureIndex, surface.lightmapIndex,
                        surface.firstIndex, surface.indexCount,
                        surface.alphaTestRef, surface.blended,
                        surface.srcBlend, surface.dstBlend, surface.cullNone,
                        false,
                    });
                    (surface.blended ? draw.anyBlended : draw.anyOpaque) = true;
                }
                gl.worldModels.push_back(std::move(draw));
            }
        }
    }

    // Sky cubemap + program.
    if (scene.skyCubemap.valid) {
        gl.worldSkyProgram = CreateWorldSkyProgram();
        if (gl.worldSkyProgram != 0) {
            gl.worldSkyMvpUniform = glGetUniformLocation(gl.worldSkyProgram, "uMvp");
            gl.worldSkyCamUniform = glGetUniformLocation(gl.worldSkyProgram, "uCamPos");
            gl.worldSkySamplerUniform = glGetUniformLocation(gl.worldSkyProgram, "uSky");
            glGenTextures(1, &gl.worldSkyTexture);
            glBindTexture(GL_TEXTURE_CUBE_MAP, gl.worldSkyTexture);
            for (int face = 0; face < 6; ++face) {
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA,
                    scene.skyCubemap.width, scene.skyCubemap.height, 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, scene.skyCubemap.faces[face].data()
                );
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }

    gl.worldName = scene.name;
    gl.worldGround = scene.ground;
    gl.worldBrushes = scene.brushes;
    gl.worldBrushPlanes = scene.brushPlanes;
    gl.camPos[0] = scene.spawnOrigin[0];
    gl.camPos[1] = scene.spawnOrigin[1];
    gl.camPos[2] = scene.spawnOrigin[2] + 55.0f;
    const float spawnGround = QueryWorldGround(
        gl.worldGround, gl.camPos[0], gl.camPos[1], scene.spawnOrigin[2] + 50.0f);
    if (spawnGround > -1e29f) {
        gl.camPos[2] = spawnGround + 60.0f;
    }
    gl.camYaw = scene.hasSpawn ? scene.spawnYaw : 0.0f;
    gl.camPitch = 0.0f;
    gl.worldReady = true;
    __android_log_print(
        ANDROID_LOG_INFO,
        kLogTag,
        "Monde GL pret: '%s' %zu runs, %zu textures, camera=(%.0f,%.0f,%.0f) yaw=%.0f",
        gl.worldName.c_str(),
        gl.worldRuns.size(),
        gl.worldTextures.size(),
        gl.camPos[0],
        gl.camPos[1],
        gl.camPos[2],
        gl.camYaw
    );
    return true;
}

// One-finger camera: left half of the screen is a virtual move stick
// (free-fly along the view direction), right half drags the view.
void UpdateWorldCamera(GlContext& gl) {
    const TouchSnapshot touch = GetTouchSnapshot();
    constexpr float kDegToRad = 0.01745329252f;
    if (touch.action == kTouchDown) {
        gl.worldTouchMode = (touch.x < gl.width * 0.5f) ? 1 : 2;
        gl.worldTouchStartX = touch.x;
        gl.worldTouchStartY = touch.y;
        gl.worldTouchLastX = touch.x;
        gl.worldTouchLastY = touch.y;
    } else if (touch.action == kTouchUp || touch.action == kTouchCancel) {
        gl.worldTouchMode = 0;
    }
    if (gl.worldTouchMode == 2 && touch.action == kTouchMove) {
        gl.camYaw -= (touch.x - gl.worldTouchLastX) * 0.22f;
        gl.camPitch -= (touch.y - gl.worldTouchLastY) * 0.22f;
        gl.camPitch = std::clamp(gl.camPitch, -89.0f, 89.0f);
        gl.worldTouchLastX = touch.x;
        gl.worldTouchLastY = touch.y;
    }
    if (gl.worldTouchMode == 1
        && (touch.action == kTouchMove || touch.action == kTouchDown)) {
        const float stickX = std::clamp((touch.x - gl.worldTouchStartX) / 140.0f, -1.0f, 1.0f);
        const float stickY = std::clamp((touch.y - gl.worldTouchStartY) / 140.0f, -1.0f, 1.0f);
        const float yaw = gl.camYaw * kDegToRad;
        // Walking: movement stays horizontal regardless of pitch.
        const float forward[2] = {std::cos(yaw), std::sin(yaw)};
        const float right[2] = {std::sin(yaw), -std::cos(yaw)};
        constexpr float kSpeedPerFrame = 4.5f; // ~270 unites/s a 60 fps (course COD)
        for (int axis = 0; axis < 2; ++axis) {
            gl.camPos[axis] += (-stickY * forward[axis] + stickX * right[axis]) * kSpeedPerFrame;
        }
    }
    // Wall collision: push the camera sphere out of the solid clipmap
    // brushes before settling on the ground.
    ResolveWorldCollision(gl.worldBrushes, gl.worldBrushPlanes, gl.camPos, 16.0f);
    // Ground follow: settle the camera onto the highest floor within step
    // range below the eyes (45-unit step-up, smoothed).
    constexpr float kEyeHeight = 60.0f;
    const float feetZ = gl.camPos[2] - kEyeHeight;
    const float floorZ = QueryWorldGround(
        gl.worldGround, gl.camPos[0], gl.camPos[1], feetZ + 45.0f);
    if (floorZ > -1e29f) {
        const float targetZ = floorZ + kEyeHeight;
        gl.camPos[2] += (targetZ - gl.camPos[2]) * 0.35f;
    }
}

void DrawWorldScene(GlContext& gl) {
    UpdateWorldCamera(gl);
    constexpr float kDegToRad = 0.01745329252f;
    const float yaw = gl.camYaw * kDegToRad;
    const float pitch = gl.camPitch * kDegToRad;
    const float f[3] = {
        std::cos(yaw) * std::cos(pitch),
        std::sin(yaw) * std::cos(pitch),
        std::sin(pitch),
    };
    const float r[3] = {std::sin(yaw), -std::cos(yaw), 0.0f};
    const float u[3] = {
        r[1] * f[2] - r[2] * f[1],
        r[2] * f[0] - r[0] * f[2],
        r[0] * f[1] - r[1] * f[0],
    };
    const float* pos = gl.camPos;
    // Column-major view matrix (rows r/u/-f, translated to the camera).
    float view[16] = {
        r[0], u[0], -f[0], 0.0f,
        r[1], u[1], -f[1], 0.0f,
        r[2], u[2], -f[2], 0.0f,
        -(r[0] * pos[0] + r[1] * pos[1] + r[2] * pos[2]),
        -(u[0] * pos[0] + u[1] * pos[1] + u[2] * pos[2]),
        f[0] * pos[0] + f[1] * pos[1] + f[2] * pos[2],
        1.0f,
    };
    const float aspect = gl.height > 0 ? static_cast<float>(gl.width) / gl.height : 1.0f;
    const float fovScale = 1.0f / std::tan(65.0f * 0.5f * kDegToRad);
    const float nearZ = 4.0f;
    const float farZ = 60000.0f;
    float projection[16] = {};
    projection[0] = fovScale / aspect;
    projection[5] = fovScale;
    projection[10] = (farZ + nearZ) / (nearZ - farZ);
    projection[11] = -1.0f;
    projection[14] = 2.0f * farZ * nearZ / (nearZ - farZ);
    float mvp[16];
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += projection[k * 4 + row] * view[column * 4 + k];
            }
            mvp[column * 4 + row] = sum;
        }
    }

    glViewport(0, 0, gl.width, gl.height);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.45f, 0.53f, 0.62f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);

    const auto bindRunTexture = [&gl](const WorldDrawRun& run) {
        GLuint texture = gl.whiteTexture;
        if (run.textureIndex >= 0
            && static_cast<size_t>(run.textureIndex) < gl.worldTextures.size()
            && gl.worldTextures[run.textureIndex] != 0) {
            texture = gl.worldTextures[run.textureIndex];
        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
    };
    const auto applyRunCull = [](bool cullNone, bool& culling) {
        if (cullNone == culling) {
            culling = !cullNone;
            if (culling) {
                glEnable(GL_CULL_FACE);
            } else {
                glDisable(GL_CULL_FACE);
            }
        }
    };

    // Empirically the retail index winding lands counter-clockwise in GL
    // clip space (CW made every one-sided wall vanish).
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    // World geometry pass (runs are sorted opaque first, then blended).
    const auto drawWorldRuns = [&](bool blendedPhase) {
        glUseProgram(gl.worldProgram);
        glUniformMatrix4fv(gl.worldMvpUniform, 1, GL_FALSE, mvp);
        glUniform1i(gl.worldTextureUniform, 0);
        glUniform1i(gl.worldLightmapUniform, 1);
        glUniform1i(gl.worldSunMaskUniform, 2);
        glUniform3fv(gl.worldSunColorUniform, 1, gl.worldSunColor);
        glBindBuffer(GL_ARRAY_BUFFER, gl.worldVbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl.worldIbo);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), nullptr);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float),
            reinterpret_cast<const void*>(3 * sizeof(float))
        );
        glVertexAttribPointer(
            2, 4, GL_FLOAT, GL_FALSE, 11 * sizeof(float),
            reinterpret_cast<const void*>(5 * sizeof(float))
        );
        glVertexAttribPointer(
            3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float),
            reinterpret_cast<const void*>(9 * sizeof(float))
        );
        float currentAlphaRef = -2.0f;
        int currentLightmap = -2;
        int currentBlendPair = -1;
        // Brush and terrain surfaces wind in opposite directions in the raw
        // data (the D3D path flips cull mode per pass); until that split is
        // ported, world geometry draws double-sided.
        glDisable(GL_CULL_FACE);
        for (const WorldDrawRun& run : gl.worldRuns) {
            if (run.sky || run.blended != blendedPhase) {
                continue;
            }
            if (blendedPhase) {
                const int blendPair = run.srcBlend * 16 + run.dstBlend;
                if (blendPair != currentBlendPair) {
                    currentBlendPair = blendPair;
                    glBlendFunc(
                        GlBlendFactor(run.srcBlend, GL_SRC_ALPHA),
                        GlBlendFactor(run.dstBlend, GL_ONE_MINUS_SRC_ALPHA)
                    );
                }
            }
            if (run.alphaTestRef != currentAlphaRef) {
                currentAlphaRef = run.alphaTestRef;
                glUniform1f(gl.worldAlphaRefUniform, currentAlphaRef);
            }
            if (run.lightmapIndex != currentLightmap) {
                currentLightmap = run.lightmapIndex;
                GLuint lightmap = gl.worldGrayTexture;
                GLuint sunMask = gl.worldBlackTexture;
                if (currentLightmap >= 0
                    && static_cast<size_t>(currentLightmap) < gl.worldLightmapTextures.size()) {
                    if (gl.worldLightmapTextures[currentLightmap] != 0) {
                        lightmap = gl.worldLightmapTextures[currentLightmap];
                    }
                    if (gl.worldSunMaskTextures[currentLightmap] != 0) {
                        sunMask = gl.worldSunMaskTextures[currentLightmap];
                    }
                }
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, lightmap);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, sunMask);
            }
            bindRunTexture(run);
            glDrawElements(
                GL_TRIANGLES,
                static_cast<GLsizei>(run.indexCount),
                GL_UNSIGNED_INT,
                reinterpret_cast<const void*>(static_cast<uintptr_t>(run.firstIndex) * 4)
            );
        }
        glDisableVertexAttribArray(3);
    };

    // Static model pass: one instanced draw per (model, surface).
    const auto drawWorldModels = [&](bool blendedPhase) {
        if (gl.worldModelProgram == 0 || gl.worldModels.empty()) {
            return;
        }
        glUseProgram(gl.worldModelProgram);
        glUniformMatrix4fv(gl.worldModelMvpUniform, 1, GL_FALSE, mvp);
        glUniform1i(gl.worldModelTextureUniform, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl.worldModelIbo);
        glBindBuffer(GL_ARRAY_BUFFER, gl.worldModelVbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), nullptr);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
            reinterpret_cast<const void*>(3 * sizeof(float))
        );
        glVertexAttribPointer(
            2, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
            reinterpret_cast<const void*>(5 * sizeof(float))
        );
        glBindBuffer(GL_ARRAY_BUFFER, gl.worldModelInstanceVbo);
        for (GLuint attrib = 3; attrib <= 6; ++attrib) {
            glEnableVertexAttribArray(attrib);
            glVertexAttribDivisor(attrib, 1);
        }
        float currentAlphaRef = -2.0f;
        bool culling = false;
        int currentBlendPair = -1;
        glDisable(GL_CULL_FACE);
        for (const WorldModelDraw& model : gl.worldModels) {
            if (!(blendedPhase ? model.anyBlended : model.anyOpaque)) {
                continue;
            }
            const uintptr_t instanceBase =
                static_cast<uintptr_t>(model.firstInstance) * 12 * sizeof(float);
            for (GLuint attrib = 3; attrib <= 6; ++attrib) {
                glVertexAttribPointer(
                    attrib, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float),
                    reinterpret_cast<const void*>(instanceBase + (attrib - 3) * 3 * sizeof(float))
                );
            }
            for (const WorldDrawRun& run : model.surfaces) {
                if (run.blended != blendedPhase) {
                    continue;
                }
                if (blendedPhase) {
                    const int blendPair = run.srcBlend * 16 + run.dstBlend;
                    if (blendPair != currentBlendPair) {
                        currentBlendPair = blendPair;
                        glBlendFunc(
                            GlBlendFactor(run.srcBlend, GL_SRC_ALPHA),
                            GlBlendFactor(run.dstBlend, GL_ONE_MINUS_SRC_ALPHA)
                        );
                    }
                }
                if (run.alphaTestRef != currentAlphaRef) {
                    currentAlphaRef = run.alphaTestRef;
                    glUniform1f(gl.worldModelAlphaRefUniform, currentAlphaRef);
                }
                applyRunCull(run.cullNone, culling);
                bindRunTexture(run);
                glDrawElementsInstanced(
                    GL_TRIANGLES,
                    static_cast<GLsizei>(run.indexCount),
                    GL_UNSIGNED_INT,
                    reinterpret_cast<const void*>(static_cast<uintptr_t>(run.firstIndex) * 4),
                    model.instanceCount
                );
            }
        }
        glDisable(GL_CULL_FACE);
        for (GLuint attrib = 3; attrib <= 6; ++attrib) {
            glVertexAttribDivisor(attrib, 0);
            glDisableVertexAttribArray(attrib);
        }
    };

    // Sky pass: drawn after opaque geometry with depth pinned to the far
    // plane, so only visible sky pixels sample the cubemap.
    const auto drawWorldSky = [&]() {
        if (gl.worldSkyProgram == 0 || gl.worldSkyTexture == 0) {
            return;
        }
        glUseProgram(gl.worldSkyProgram);
        glUniformMatrix4fv(gl.worldSkyMvpUniform, 1, GL_FALSE, mvp);
        glUniform3fv(gl.worldSkyCamUniform, 1, gl.camPos);
        glUniform1i(gl.worldSkySamplerUniform, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, gl.worldSkyTexture);
        glBindBuffer(GL_ARRAY_BUFFER, gl.worldVbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl.worldIbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), nullptr);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        for (const WorldDrawRun& run : gl.worldRuns) {
            if (!run.sky) {
                continue;
            }
            glDrawElements(
                GL_TRIANGLES,
                static_cast<GLsizei>(run.indexCount),
                GL_UNSIGNED_INT,
                reinterpret_cast<const void*>(static_cast<uintptr_t>(run.firstIndex) * 4)
            );
        }
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    };

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    drawWorldRuns(false);
    drawWorldModels(false);
    drawWorldSky();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    drawWorldRuns(true);
    drawWorldModels(true);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);

    // Frame pacing telemetry: a hidden-app freeze looks identical to a slow
    // GPU from the outside, so log the world frame rate periodically.
    static uint32_t frameCounter = 0;
    static int64_t windowStartNs = 0;
    struct timespec now{};
    clock_gettime(CLOCK_MONOTONIC, &now);
    const int64_t nowNs = now.tv_sec * 1000000000ll + now.tv_nsec;
    if (windowStartNs == 0) {
        windowStartNs = nowNs;
    }
    if (++frameCounter >= 120) {
        __android_log_print(
            ANDROID_LOG_INFO,
            kLogTag,
            "Monde: %.1f fps (%u frames)",
            frameCounter * 1e9 / static_cast<double>(nowNs - windowStartNs),
            frameCounter
        );
        frameCounter = 0;
        windowStartNs = nowNs;
    }
}

// Builds the GL resources for the COD4 menu scene once the startup probe
// has published the loaded zone.
bool InitMenuScene(GlContext& gl) {
    const std::vector<std::shared_ptr<const KisakZoneLoadResult>> zones = GetKisakMenuZones();
    if (zones.empty()) {
        return false;
    }
    const uint32_t zoneGeneration = GetKisakMenuZoneGeneration();
    gl.menuInitTried = true;
    const bool suppressSceneLog = gl.suppressNextMenuSceneLog;

    KisakMenuScene scene = BuildMenuScene(zones, gl.menuTargetName);
    if (gl.menuTargetName == "main" && scene.error.empty() && scene.quads.empty()) {
        if (!suppressSceneLog) {
            __android_log_print(
                ANDROID_LOG_INFO,
                kLogTag,
                "Menu 'main' vide, fallback vers main_text"
            );
        }
        gl.menuTargetName = "main_text";
        scene = BuildMenuScene(zones, "main_text");
    }
    if (!suppressSceneLog) {
        __android_log_print(
            ANDROID_LOG_INFO,
            kLogTag,
            "Menu scene target='%s': %s",
            gl.menuTargetName.c_str(),
            DescribeMenuScene(scene).c_str()
        );
    }
    if (!scene.error.empty() || scene.quads.empty()) {
        if (gl.menuTargetName != "main") {
            __android_log_print(
                ANDROID_LOG_WARN,
                kLogTag,
                "Menu cible '%s' invalide, retour au menu principal",
                gl.menuTargetName.c_str()
            );
            gl.menuTargetName = "main";
            scene = BuildMenuScene(zones, "main");
            if (scene.error.empty() && scene.quads.empty()) {
                scene = BuildMenuScene(zones, "main_text");
            }
        }
    }
    if (!scene.error.empty() || scene.quads.empty()) {
        return false;
    }

    // Menu open protocol: run onOpen once per navigation (focusFirst runs the
    // first focusable item's onFocus, which drives the hide/show groups),
    // then rebuild the scene against the updated UI state.
    if (scene.menuZone != nullptr && scene.menuName != gl.lastOnOpenMenu) {
        gl.lastOnOpenMenu = scene.menuName;
        ResetKisakMenuForOpen(*scene.menuZone, scene.menuRef);
        RunKisakMenuOnOpen(*scene.menuZone, scene.menuRef);
        scene = BuildMenuScene(zones, scene.menuName);
        if (!scene.error.empty() || scene.quads.empty()) {
            return false;
        }
    }
    gl.menuSceneZones = zones;
    gl.menuSceneZone = scene.menuZone;
    gl.menuSceneMenuRef = scene.menuRef;
    gl.menuSceneDirty = false;
    gl.menuTimeDependent = scene.timeDependentExpressions;
    gl.lastMenuTimeRefreshFrame = g_frameId;

    gl.menuProgram = CreateMenuProgram();
    if (gl.menuProgram == 0) {
        return false;
    }
    gl.menuScaleUniform = glGetUniformLocation(gl.menuProgram, "uScale");
    gl.menuOffsetUniform = glGetUniformLocation(gl.menuProgram, "uOffset");
    gl.menuTextureUniform = glGetUniformLocation(gl.menuProgram, "uTexture");

    const uint8_t white[4] = {255, 255, 255, 255};
    gl.whiteTexture = UploadRgbaTexture(white, 1, 1);
    gl.menuTextures.clear();
    for (const std::string& imageName : scene.textureImages) {
        gl.menuTextures.push_back(CreateMenuTexture(gl, imageName));
    }

    // Interleaved vertices: pos(2) uv(2) color(4), two triangles per quad,
    // batched into consecutive same-texture runs.
    std::vector<float> vertices;
    gl.menuRuns.clear();
    for (const KisakMenuQuad& quad : scene.quads) {
        const float x1 = quad.x + quad.w;
        const float y1 = quad.y + quad.h;
        const float corners[6][4] = {
            {quad.x, quad.y, quad.u0, quad.v0},
            {quad.x, y1, quad.u0, quad.v1},
            {x1, quad.y, quad.u1, quad.v0},
            {x1, quad.y, quad.u1, quad.v0},
            {quad.x, y1, quad.u0, quad.v1},
            {x1, y1, quad.u1, quad.v1},
        };
        const GLint firstVertex = static_cast<GLint>(vertices.size() / 8);
        for (const float* corner : corners) {
            vertices.insert(vertices.end(), {corner[0], corner[1], corner[2], corner[3],
                quad.color[0], quad.color[1], quad.color[2], quad.color[3]});
        }
        if (!gl.menuRuns.empty() && gl.menuRuns.back().textureIndex == quad.textureIndex) {
            gl.menuRuns.back().vertexCount += 6;
        } else {
            gl.menuRuns.push_back({quad.textureIndex, firstVertex, 6});
        }
    }

    glGenBuffers(1, &gl.menuVbo);
    glBindBuffer(GL_ARRAY_BUFFER, gl.menuVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
        vertices.data(),
        GL_STATIC_DRAW
    );
    glGenBuffers(1, &gl.menuCursorVbo);

    gl.menuName = scene.menuName;
    gl.menuHitboxes = scene.hitboxes;
    gl.menuCanvasW = scene.canvasWidth;
    gl.menuCanvasH = scene.canvasHeight;
    gl.menuZoneGeneration = zoneGeneration;
    gl.menuReady = true;
    if (!suppressSceneLog) {
        __android_log_print(
            ANDROID_LOG_INFO,
            kLogTag,
            "Menu GL pret: '%s' %zu runs, %zu textures, %zu zones tactiles",
            gl.menuName.c_str(),
            gl.menuRuns.size(),
            gl.menuTextures.size(),
            gl.menuHitboxes.size()
        );
    }
    gl.suppressNextMenuSceneLog = false;
    return true;
}

bool SurfaceToMenuCanvas(
    const GlContext& gl,
    float fit,
    float offsetX,
    float offsetY,
    const TouchSnapshot& touch,
    float& canvasX,
    float& canvasY
) {
    if (fit <= 0.0f) {
        return false;
    }
    canvasX = std::clamp((touch.x - offsetX) / fit, 0.0f, gl.menuCanvasW);
    canvasY = std::clamp((touch.y - offsetY) / fit, 0.0f, gl.menuCanvasH);
    return true;
}

int FindMenuHitbox(const GlContext& gl, float canvasX, float canvasY) {
    for (size_t i = gl.menuHitboxes.size(); i > 0; --i) {
        const size_t index = i - 1;
        const KisakMenuHitbox& hitbox = gl.menuHitboxes[index];
        if (canvasX >= hitbox.x && canvasX <= hitbox.x + hitbox.w
            && canvasY >= hitbox.y && canvasY <= hitbox.y + hitbox.h) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

void PushMenuVertex(
    std::vector<float>& vertices,
    float x,
    float y,
    float red,
    float green,
    float blue,
    float alpha
) {
    vertices.insert(vertices.end(), {x, y, 0.0f, 0.0f, red, green, blue, alpha});
}

void PushMenuQuad(
    std::vector<float>& vertices,
    float x,
    float y,
    float w,
    float h,
    float red,
    float green,
    float blue,
    float alpha
) {
    const float x1 = x + w;
    const float y1 = y + h;
    PushMenuVertex(vertices, x, y, red, green, blue, alpha);
    PushMenuVertex(vertices, x, y1, red, green, blue, alpha);
    PushMenuVertex(vertices, x1, y, red, green, blue, alpha);
    PushMenuVertex(vertices, x1, y, red, green, blue, alpha);
    PushMenuVertex(vertices, x, y1, red, green, blue, alpha);
    PushMenuVertex(vertices, x1, y1, red, green, blue, alpha);
}

void DrawMenuTouchFeedback(GlContext& gl, float fit, float offsetX, float offsetY) {
    const TouchSnapshot touch = GetTouchSnapshot();
    if (touch.action == kTouchCancel || gl.menuCursorVbo == 0) {
        return;
    }

    float canvasX = 0.0f;
    float canvasY = 0.0f;
    if (!SurfaceToMenuCanvas(gl, fit, offsetX, offsetY, touch, canvasX, canvasY)) {
        return;
    }

    std::vector<float> vertices;
    const int selected = FindMenuHitbox(gl, canvasX, canvasY);
    if (touch.action == kTouchDown) {
        gl.menuPressedHitbox = selected;
    }
    if (selected >= 0) {
        const KisakMenuHitbox& hitbox = gl.menuHitboxes[static_cast<size_t>(selected)];
        if (gl.menuLoggedHitbox != selected) {
            __android_log_print(
                ANDROID_LOG_INFO,
                kLogTag,
                "Menu touch: '%s' item=%s canvas=(%.1f,%.1f)",
                hitbox.text.c_str(),
                hitbox.name.empty() ? "<sans nom>" : hitbox.name.c_str(),
                canvasX,
                canvasY
            );
            gl.menuLoggedHitbox = selected;
        }

        // Touch-down/move gives the item focus (its onFocus script drives
        // the hide/show groups and highlight localvars).
        if ((touch.action == kTouchDown || touch.action == kTouchMove)
            && gl.menuSceneZone != nullptr && hitbox.itemRef != 0
            && KisakMenuFocusItem(*gl.menuSceneZone, gl.menuSceneMenuRef, hitbox.itemRef)) {
            gl.menuSceneDirty = true;
        }

        if (touch.action == kTouchUp && gl.menuActivatedTouchSerial != touch.serial
            && (gl.menuPressedHitbox == selected || gl.menuPressedHitbox < 0)) {
            KisakMenuScriptResult script;
            bool itemStateChanged = false;
            if (gl.menuSceneZone != nullptr && hitbox.itemRef != 0) {
                itemStateChanged = KisakMenuItemActivate(
                    *gl.menuSceneZone,
                    hitbox.itemRef,
                    canvasX,
                    canvasY
                );
                if (!hitbox.action.empty()) {
                    script = RunKisakMenuScript(*gl.menuSceneZone, gl.menuSceneMenuRef, hitbox.action);
                }
            }
            const std::string launchCommand = ConsumeKisakMenuLaunchCommand();
            __android_log_print(
                ANDROID_LOG_INFO,
                kLogTag,
                "Menu activate: '%s' item=%s open=%s closeSelf=%d dvarChange=%d launch=%s action=%s",
                hitbox.text.c_str(),
                hitbox.name.empty() ? "<sans nom>" : hitbox.name.c_str(),
                script.openTarget.empty() ? "<aucun>" : script.openTarget.c_str(),
                script.closeSelf ? 1 : 0,
                itemStateChanged ? 1 : 0,
                launchCommand.empty() ? "<aucun>" : launchCommand.c_str(),
                hitbox.action.empty() ? "<aucune>" : hitbox.action.c_str()
            );
            if (!launchCommand.empty()) {
                __android_log_print(
                    ANDROID_LOG_INFO,
                    kLogTag,
                    "Menu launch requested: %s",
                    launchCommand.c_str()
                );
                gl.menuActivatedTouchSerial = touch.serial;
                ConsumeTouchSnapshot(touch.serial);
                StartWorldLoad(LaunchCommandMapName(launchCommand));
            }
            if (!script.openTarget.empty()) {
                QueueMenuNavigation(gl, script.openTarget, true);
            } else if (script.closeSelf) {
                QueueMenuBack(gl);
            } else if (itemStateChanged || script.stateChanged) {
                gl.menuSceneDirty = true;
            }
            gl.menuActivatedTouchSerial = touch.serial;
            gl.menuPressedHitbox = -1;
            ConsumeTouchSnapshot(touch.serial);
        }

        const float pad = 6.0f;
        const float highlightX = std::max(hitbox.x - pad, 0.0f);
        const float highlightY = std::max(hitbox.y - 2.0f, 0.0f);
        const float highlightW = std::min(hitbox.w + pad * 2.0f, gl.menuCanvasW - highlightX);
        const float highlightH = std::min(hitbox.h + 4.0f, gl.menuCanvasH - highlightY);
        PushMenuQuad(vertices, highlightX, highlightY, highlightW, highlightH, 0.86f, 0.74f, 0.48f, 0.23f);

        const float arrowX = std::max(highlightX - 15.0f, 0.0f);
        const float arrowY = highlightY + highlightH * 0.5f;
        PushMenuVertex(vertices, arrowX, arrowY - 7.0f, 1.0f, 1.0f, 1.0f, 0.92f);
        PushMenuVertex(vertices, arrowX, arrowY + 7.0f, 1.0f, 1.0f, 1.0f, 0.92f);
        PushMenuVertex(vertices, arrowX + 11.0f, arrowY, 1.0f, 1.0f, 1.0f, 0.92f);
    } else {
        gl.menuLoggedHitbox = -1;
    }

    if (touch.action == kTouchDown || touch.action == kTouchMove) {
        const float size = 18.0f;
        PushMenuVertex(vertices, canvasX, canvasY, 1.0f, 1.0f, 1.0f, 0.95f);
        PushMenuVertex(vertices, canvasX, canvasY + size, 1.0f, 1.0f, 1.0f, 0.95f);
        PushMenuVertex(vertices, canvasX + size * 0.72f, canvasY + size * 0.55f, 1.0f, 1.0f, 1.0f, 0.95f);
    }

    if (vertices.empty()) {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, gl.menuCursorVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
        vertices.data(),
        GL_DYNAMIC_DRAW
    );
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        reinterpret_cast<const void*>(2 * sizeof(float))
    );
    glVertexAttribPointer(
        2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        reinterpret_cast<const void*>(4 * sizeof(float))
    );
    glBindTexture(GL_TEXTURE_2D, gl.whiteTexture);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 8));
}

void DrawMenuScene(GlContext& gl) {
    // Fit the widescreen virtual canvas into the surface, centered, and
    // clip to it (the animated fog layers extend far past the canvas).
    const float fit = std::min(gl.width / gl.menuCanvasW, gl.height / gl.menuCanvasH);
    const float offsetX = (gl.width - gl.menuCanvasW * fit) * 0.5f;
    const float offsetY = (gl.height - gl.menuCanvasH * fit) * 0.5f;
    const auto canvasW = static_cast<GLsizei>(gl.menuCanvasW * fit);
    const auto canvasH = static_cast<GLsizei>(gl.menuCanvasH * fit);
    glEnable(GL_SCISSOR_TEST);
    glScissor(
        static_cast<GLint>(offsetX),
        static_cast<GLint>(gl.height - offsetY - canvasH),
        canvasW,
        canvasH
    );

    glUseProgram(gl.menuProgram);
    glUniform2f(gl.menuScaleUniform, 2.0f * fit / gl.width, -2.0f * fit / gl.height);
    glUniform2f(
        gl.menuOffsetUniform,
        -1.0f + 2.0f * offsetX / gl.width,
        1.0f - 2.0f * offsetY / gl.height
    );
    glUniform1i(gl.menuTextureUniform, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindBuffer(GL_ARRAY_BUFFER, gl.menuVbo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        reinterpret_cast<const void*>(2 * sizeof(float))
    );
    glVertexAttribPointer(
        2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        reinterpret_cast<const void*>(4 * sizeof(float))
    );

    glActiveTexture(GL_TEXTURE0);
    for (const MenuDrawRun& run : gl.menuRuns) {
        GLuint texture = gl.whiteTexture;
        if (run.textureIndex >= 0) {
            if (static_cast<size_t>(run.textureIndex) >= gl.menuTextures.size()
                || gl.menuTextures[run.textureIndex] == 0) {
                continue; // image missing or undecodable
            }
            texture = gl.menuTextures[run.textureIndex];
        }
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawArrays(GL_TRIANGLES, run.firstVertex, run.vertexCount);
    }
    DrawMenuTouchFeedback(gl, fit, offsetX, offsetY);

    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
}

bool InitGlPipeline(GlContext& gl) {
    gl.program = CreateTexturedQuadProgram();
    if (gl.program == 0) {
        return false;
    }

    // Unit quad as a triangle strip; v=0 maps to the top row of the IWI mip.
    constexpr float vertices[] = {
        -1.0f,  1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 1.0f,
    };

    glGenBuffers(1, &gl.vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gl.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    gl.phaseUniform = glGetUniformLocation(gl.program, "uPhase");
    gl.textureUniform = glGetUniformLocation(gl.program, "uTexture");
    gl.scaleUniform = glGetUniformLocation(gl.program, "uScale");
    gl.assetTexture = CreateAssetTexture(gl.texWidth, gl.texHeight);

    __android_log_print(
        ANDROID_LOG_INFO,
        kLogTag,
        "OpenGL ES shader pipeline ready: program=%u vbo=%u texture=%u",
        gl.program,
        gl.vertexBuffer,
        gl.assetTexture
    );
    return true;
}

void ShutdownEgl(GlContext& gl) {
    if (gl.display != EGL_NO_DISPLAY) {
        DestroyWorldSceneResources(gl);
        if (gl.vertexBuffer != 0) {
            glDeleteBuffers(1, &gl.vertexBuffer);
        }
        if (gl.assetTexture != 0) {
            glDeleteTextures(1, &gl.assetTexture);
        }
        if (gl.program != 0) {
            glDeleteProgram(gl.program);
        }
        if (gl.menuVbo != 0) {
            glDeleteBuffers(1, &gl.menuVbo);
        }
        if (gl.menuCursorVbo != 0) {
            glDeleteBuffers(1, &gl.menuCursorVbo);
        }
        for (GLuint texture : gl.menuTextures) {
            if (texture != 0 && texture != gl.whiteTexture) {
                glDeleteTextures(1, &texture);
            }
        }
        if (gl.whiteTexture != 0) {
            glDeleteTextures(1, &gl.whiteTexture);
        }
        if (gl.menuProgram != 0) {
            glDeleteProgram(gl.menuProgram);
        }
        eglMakeCurrent(gl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (gl.context != EGL_NO_CONTEXT) {
            eglDestroyContext(gl.display, gl.context);
        }
        if (gl.surface != EGL_NO_SURFACE) {
            eglDestroySurface(gl.display, gl.surface);
        }
        eglTerminate(gl.display);
    }
    gl = {};
}

void DrawGlRect(int x, int y, int width, int height, float red, float green, float blue) {
    glScissor(x, y, width, height);
    glClearColor(red, green, blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

KisakNativeRendererState DrawKisakGlFrame(GlContext& gl) {
    KisakNativeRendererState state;
    state.width = gl.width;
    state.height = gl.height;
    state.frameId = ++g_frameId;

    const bool fsReady = AndroidFsInitialized();
    const float pulse = static_cast<float>(state.frameId % 90) / 90.0f;
    glViewport(0, 0, gl.width, gl.height);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, gl.width, gl.height);
    glClearColor(0.025f + pulse * 0.02f, 0.035f, 0.028f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // The world viewer owns the frame while a mission world is loaded; the
    // Android back button returns to the menu.
    if (!gl.worldReady) {
        std::shared_ptr<KisakWorldScene> pendingScene;
        {
            std::lock_guard<std::mutex> lock(g_worldLoadMutex);
            pendingScene.swap(g_pendingWorldScene);
        }
        if (pendingScene != nullptr && pendingScene->error.empty()) {
            InitWorldScene(gl, *pendingScene);
        }
    }
    if (gl.worldReady) {
        if (g_menuBackRequested.exchange(false, std::memory_order_acq_rel)) {
            __android_log_print(ANDROID_LOG_INFO, kLogTag, "Retour au menu depuis le monde");
            DestroyWorldSceneResources(gl);
            ConsumeTouchSnapshot(GetTouchSnapshot().serial);
        } else {
            DrawWorldScene(gl);
            state.drewFrame = eglSwapBuffers(gl.display, gl.surface) == EGL_TRUE;
            return state;
        }
    }

    // Render the loaded COD4 menu once the zone is published; the compass
    // diagnostic stays as the fallback scene.
    const uint32_t menuZoneGeneration = GetKisakMenuZoneGeneration();
    if (gl.menuReady && g_menuBackRequested.exchange(false, std::memory_order_acq_rel)) {
        QueueMenuBack(gl);
    }
    if (gl.menuReady && g_menuTextInputChanged.exchange(false, std::memory_order_acq_rel)) {
        gl.menuSceneDirty = true;
    }
    if (gl.menuReady && !gl.pendingMenuTargetName.empty()) {
        const std::string target = gl.pendingMenuTargetName;
        gl.pendingMenuTargetName.clear();
        __android_log_print(
            ANDROID_LOG_INFO,
            kLogTag,
            "Menu navigation: '%s' -> '%s'",
            gl.menuName.c_str(),
            target.c_str()
        );
        DestroyMenuSceneResources(gl);
        gl.menuTargetName = target;
        ConsumeTouchSnapshot(GetTouchSnapshot().serial);
    }
    if (gl.menuReady && gl.menuZoneGeneration != menuZoneGeneration) {
        __android_log_print(
            ANDROID_LOG_INFO,
            kLogTag,
            "Menu zones maj: generation %u -> %u, reconstruction",
            gl.menuZoneGeneration,
            menuZoneGeneration
        );
        DestroyMenuSceneResources(gl);
    }
    if (gl.menuReady && gl.menuTimeDependent
        && state.frameId - gl.lastMenuTimeRefreshFrame >= 10) {
        gl.lastMenuTimeRefreshFrame = state.frameId;
        gl.suppressNextMenuSceneLog = true;
        gl.menuSceneDirty = true;
    }
    // UI scripts (focus, actions) changed dvars/localvars/visibility:
    // rebuild the scene against the new state (textures come from the cache).
    if (gl.menuReady && gl.menuSceneDirty) {
        gl.menuSceneDirty = false;
        DestroyMenuSceneResources(gl);
    }
    if (!gl.menuReady && (!gl.menuInitTried || state.frameId % 60 == 0)) {
        InitMenuScene(gl);
    }
    if (gl.menuReady) {
        glDisable(GL_SCISSOR_TEST);
        DrawMenuScene(gl);
        state.drewFrame = eglSwapBuffers(gl.display, gl.surface) == EGL_TRUE;
        return state;
    }

    const int bandWidth = gl.width * 3 / 4;
    const int bandLeft = gl.width / 8;
    const int bandBottom = gl.height / 2 - 24;
    DrawGlRect(bandLeft, bandBottom + 58, bandWidth, 10, 0.85f, 0.69f, 0.37f);
    DrawGlRect(bandLeft, bandBottom + 24, bandWidth, 18, 0.20f, 0.24f, 0.17f);
    DrawGlRect(
        bandLeft,
        bandBottom + 24,
        fsReady ? bandWidth : bandWidth / 3,
        18,
        fsReady ? 0.46f : 0.68f,
        fsReady ? 0.56f : 0.26f,
        fsReady ? 0.34f : 0.22f
    );
    DrawGlRect(bandLeft, bandBottom - 24, bandWidth, 10, 0.85f, 0.69f, 0.37f);

    const int sweepWidth = std::max(20, gl.width / 18);
    const int sweepX = bandLeft + static_cast<int>((bandWidth - sweepWidth) * pulse);
    DrawGlRect(sweepX, bandBottom - 70, sweepWidth, 18, 0.90f, 0.72f, 0.30f);
    glDisable(GL_SCISSOR_TEST);

    // Aspect-correct quad covering 80% of the screen height.
    const float texAspect = static_cast<float>(gl.texWidth) / static_cast<float>(std::max(1, gl.texHeight));
    const float screenAspect = static_cast<float>(gl.width) / static_cast<float>(std::max(1, gl.height));
    float scaleY = 0.8f;
    float scaleX = scaleY * texAspect / screenAspect;
    if (scaleX > 0.95f) {
        scaleY *= 0.95f / scaleX;
        scaleX = 0.95f;
    }

    glUseProgram(gl.program);
    glUniform1f(gl.phaseUniform, pulse);
    glUniform2f(gl.scaleUniform, scaleX, scaleY);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gl.assetTexture);
    glUniform1i(gl.textureUniform, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindBuffer(GL_ARRAY_BUFFER, gl.vertexBuffer);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        reinterpret_cast<const void*>(2 * sizeof(float))
    );
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glDisable(GL_BLEND);

    state.drewFrame = eglSwapBuffers(gl.display, gl.surface) == EGL_TRUE;
    return state;
}

void RenderLoopMain(ANativeWindow* window) {
    uint32_t localFrames = 0;
    GlContext gl;
    const bool eglInitialized = InitEgl(window, gl);
    const bool shaderPipelineReady = eglInitialized && InitGlPipeline(gl);
    if (eglInitialized && !shaderPipelineReady) {
        ShutdownEgl(gl);
    }
    const bool eglReady = eglInitialized && shaderPipelineReady;
    const char* backend = eglReady ? "EGL-GLES3" : "software";

    while (!g_stopRenderLoop.load(std::memory_order_acquire)) {
        const KisakNativeRendererState state = eglReady
            ? DrawKisakGlFrame(gl)
            : DrawKisakNativeFrame(window);
        if (state.drewFrame) {
            ++localFrames;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    if (eglReady) {
        ShutdownEgl(gl);
    }
    __android_log_print(
        ANDROID_LOG_INFO,
        kLogTag,
        "Native render loop stopped after %u frames backend=%s",
        localFrames,
        backend
    );
    ANativeWindow_release(window);
}
} // namespace

KisakNativeRendererState DrawKisakNativeFrame(ANativeWindow* window) {
    KisakNativeRendererState state;
    if (window == nullptr) {
        return state;
    }

    ANativeWindow_setBuffersGeometry(window, 0, 0, WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer buffer{};
    if (ANativeWindow_lock(window, &buffer, nullptr) != 0) {
        return state;
    }

    state.width = buffer.width;
    state.height = buffer.height;
    state.frameId = ++g_frameId;

    auto* pixels = static_cast<uint32_t*>(buffer.bits);
    DrawVignette(pixels, buffer.stride, buffer.width, buffer.height);
    DrawDiagnosticBars(
        pixels,
        buffer.stride,
        buffer.width,
        buffer.height,
        AndroidFsInitialized(),
        state.frameId
    );

    ANativeWindow_unlockAndPost(window);
    state.drewFrame = true;
    return state;
}

bool StartKisakNativeRenderLoop(ANativeWindow* window) {
    if (window == nullptr) {
        return false;
    }

    StopKisakNativeRenderLoop();

    ANativeWindow_acquire(window);
    g_stopRenderLoop.store(false, std::memory_order_release);
    {
        std::lock_guard<std::mutex> lock(g_renderThreadMutex);
        g_renderThread = std::thread(RenderLoopMain, window);
    }

    __android_log_print(ANDROID_LOG_INFO, kLogTag, "Native render loop started");
    return true;
}

void StopKisakNativeRenderLoop() {
    std::thread threadToJoin;
    {
        std::lock_guard<std::mutex> lock(g_renderThreadMutex);
        if (!g_renderThread.joinable()) {
            return;
        }
        g_stopRenderLoop.store(true, std::memory_order_release);
        threadToJoin = std::move(g_renderThread);
    }

    threadToJoin.join();
}

uint32_t KisakNativeRenderedFrameCount() {
    return g_frameId;
}

void SetKisakNativeTouch(float x, float y, int action) {
    std::lock_guard<std::mutex> lock(g_touchMutex);
    g_touchX = x;
    g_touchY = y;
    g_touchAction = std::clamp(action, static_cast<int>(kTouchDown), static_cast<int>(kTouchCancel));
    ++g_touchSerial;
}

bool SetKisakNativeKey(int unicodeChar, int keyCode) {
    if (KisakMenuTextInput(unicodeChar, keyCode)) {
        __android_log_print(
            ANDROID_LOG_INFO,
            kLogTag,
            "Menu key accepted: unicode=%d keyCode=%d",
            unicodeChar,
            keyCode
        );
        g_menuTextInputChanged.store(true, std::memory_order_release);
        return true;
    }
    return false;
}

void RequestKisakMenuBack() {
    g_menuBackRequested.store(true, std::memory_order_release);
}
