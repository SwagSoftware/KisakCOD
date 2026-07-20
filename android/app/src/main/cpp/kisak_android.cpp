#include <jni.h>
#include <android/log.h>
#include <android/native_window_jni.h>

#include "kisak_android_native_renderer.h"
#include "kisak_menu_scene_android.h"
#include "kisak_menu_state_android.h"
#include "kisak_platform_android.h"

#include <sstream>
#include <string>

namespace {
constexpr const char* kLogTag = "KisakCODAndroid";
int g_surfaceWidth = 0;
int g_surfaceHeight = 0;

std::string ToString(JNIEnv* env, jstring value) {
    if (value == nullptr) {
        return {};
    }
    const char* chars = env->GetStringUTFChars(value, nullptr);
    if (chars == nullptr) {
        return {};
    }
    std::string result(chars);
    env->ReleaseStringUTFChars(value, chars);
    return result;
}
} // namespace

extern "C" JNIEXPORT jstring JNICALL
Java_com_kisakcod_android_MainActivity_nativeBootstrapStatus(
    JNIEnv* env,
    jclass,
    jstring dataDirectory
) {
    const std::string dataPath = ToString(env, dataDirectory);
    __android_log_print(ANDROID_LOG_INFO, kLogTag, "Bootstrap status requested. dataPath=%s", dataPath.c_str());
    SetKisakMenuStorageRoot(dataPath);
    ClearKisakMenuZones();
    const KisakDataProbe probe = ProbeCod4DataDirectory(dataPath);

    std::ostringstream out;
    out << "NDK bridge charge correctement.\\n"
        << "ABI: "
#if defined(__aarch64__)
        << "arm64-v8a"
#elif defined(__arm__)
        << "armeabi-v7a"
#elif defined(__x86_64__)
        << "x86_64"
#elif defined(__i386__)
        << "x86"
#else
        << "unknown"
#endif
        << "\\n\\n"
        << probe.message;

    return env->NewStringUTF(out.str().c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_kisakcod_android_MainActivity_nativeSurfaceCreated(
    JNIEnv* env,
    jclass,
    jobject surface
) {
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (window == nullptr) {
        __android_log_print(ANDROID_LOG_WARN, kLogTag, "Surface created but ANativeWindow is null");
        return;
    }
    g_surfaceWidth = ANativeWindow_getWidth(window);
    g_surfaceHeight = ANativeWindow_getHeight(window);
    const bool loopStarted = StartKisakNativeRenderLoop(window);
    __android_log_print(
        ANDROID_LOG_INFO,
        kLogTag,
        "Surface created: %dx%d renderLoop=%d frame=%u",
        g_surfaceWidth,
        g_surfaceHeight,
        loopStarted ? 1 : 0,
        KisakNativeRenderedFrameCount()
    );
    ANativeWindow_release(window);
}

extern "C" JNIEXPORT void JNICALL
Java_com_kisakcod_android_MainActivity_nativeSurfaceChanged(
    JNIEnv* env,
    jclass,
    jobject surface,
    jint width,
    jint height
) {
    g_surfaceWidth = width;
    g_surfaceHeight = height;
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (window == nullptr) {
        __android_log_print(ANDROID_LOG_WARN, kLogTag, "Surface changed but ANativeWindow is null");
        return;
    }
    const bool loopStarted = StartKisakNativeRenderLoop(window);
    __android_log_print(
        ANDROID_LOG_INFO,
        kLogTag,
        "Surface changed: %dx%d renderLoop=%d frame=%u",
        g_surfaceWidth,
        g_surfaceHeight,
        loopStarted ? 1 : 0,
        KisakNativeRenderedFrameCount()
    );
    ANativeWindow_release(window);
}

extern "C" JNIEXPORT void JNICALL
Java_com_kisakcod_android_MainActivity_nativeSurfaceDestroyed(
    JNIEnv*,
    jclass
) {
    StopKisakNativeRenderLoop();
    __android_log_print(ANDROID_LOG_INFO, kLogTag, "Surface destroyed: last=%dx%d", g_surfaceWidth, g_surfaceHeight);
    g_surfaceWidth = 0;
    g_surfaceHeight = 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_kisakcod_android_MainActivity_nativeTouch(
    JNIEnv*,
    jclass,
    jfloat x,
    jfloat y,
    jint action
) {
    SetKisakNativeTouch(x, y, action);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_kisakcod_android_MainActivity_nativeKey(
    JNIEnv*,
    jclass,
    jint unicodeChar,
    jint keyCode
) {
    return SetKisakNativeKey(unicodeChar, keyCode) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_kisakcod_android_MainActivity_nativeBack(
    JNIEnv*,
    jclass
) {
    RequestKisakMenuBack();
}
