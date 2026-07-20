#pragma once

#include <android/native_window.h>

#include <cstdint>

struct KisakNativeRendererState {
    int width = 0;
    int height = 0;
    uint32_t frameId = 0;
    bool drewFrame = false;
};

KisakNativeRendererState DrawKisakNativeFrame(ANativeWindow* window);
bool StartKisakNativeRenderLoop(ANativeWindow* window);
void StopKisakNativeRenderLoop();
uint32_t KisakNativeRenderedFrameCount();
void SetKisakNativeTouch(float x, float y, int action);
bool SetKisakNativeKey(int unicodeChar, int keyCode);
void RequestKisakMenuBack();
