// EagleView mod skeleton.
// Loads, logs, and proves the core math is compiled in.
// The three HOOK slots below are what is still missing.
#include <cstdio>

#include "EagleView.hpp"

#ifdef __ANDROID__
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "EagleView", __VA_ARGS__)
#else
#define LOGI(...) do { std::printf(__VA_ARGS__); std::printf("\n"); } while (0)
#endif

namespace {

eagleview::Settings gSettings;
float gOpacity = 0.0f;

// ---- HOOK SLOTS (fill these once game hooks are known) -------------------
// 1) Player state: height (y), facing yaw in degrees, gliding or not.
struct PlayerState { float y; float yawDeg; bool gliding; };
bool readPlayerState(PlayerState&) { return false; }   // TODO hook

// 2) Map pixels: RGBA image of explored terrain, 1 pixel per block.
bool getMapTexture(void** /*texture*/) { return false; } // TODO hook

// 3) Draw: draw the texture as a quad each frame with the given opacity.
void drawQuad(void*, const eagleview::Vec2 (&/*corners*/)[4], float /*opacity*/) {} // TODO hook
// --------------------------------------------------------------------------

// Call once per frame once the hooks exist.
void onFrame(float dtSeconds, float screenW, float screenH) {
    PlayerState p;
    if (!readPlayerState(p)) return;

    float target = eagleview::targetOpacity(p.y, p.gliding, gSettings);
    gOpacity = eagleview::stepOpacity(gOpacity, target, dtSeconds, gSettings);
    if (gOpacity < 0.01f) return;

    void* tex = nullptr;
    if (!getMapTexture(&tex)) return;

    float tilt = eagleview::tiltDegrees(p.y, gSettings);
    float half = (screenH < screenW ? screenH : screenW) * 0.35f;
    auto q = eagleview::projectMapQuad({screenW * 0.5f, screenH * 0.5f}, half, p.yawDeg, tilt, gSettings);
    const eagleview::Vec2 corners[4] = {q[0], q[1], q[2], q[3]};
    drawQuad(tex, corners, gOpacity);
}

}  // namespace

__attribute__((constructor)) static void onLoad() {
    LOGI("EagleView loaded (skeleton, hooks not connected yet)");
    for (float y : {64.0f, 90.0f, 115.0f, 140.0f}) {
        LOGI("  y=%.0f -> opacity %.2f, tilt %.0f deg", y,
             eagleview::targetOpacity(y, false, gSettings),
             eagleview::tiltDegrees(y, gSettings));
    }
    (void)&onFrame;
}
