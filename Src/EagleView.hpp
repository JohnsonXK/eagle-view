// EagleView core math - no game hooks, no external libraries.
// 1) altitude -> opacity (with smooth fade over time)
// 2) flat 2D map -> tilted/rotated 4-point quad (for AddImageQuad-style drawing)
#pragma once
#include <algorithm>
#include <array>
#include <cmath>

namespace eagleview {

struct Vec2 {
    float x = 0, y = 0;
};

struct Settings {
    float startHeight = 90.0f;   // map begins to appear
    float fullHeight = 140.0f;   // map fully visible
    float maxOpacity = 0.8f;     // never fully hide the world behind it
    float glideBoost = 0.35f;    // minimum visibility while gliding
    float fadeSpeed = 6.0f;      // higher = faster fade
    float groundTiltDeg = 65.0f; // tilt at start height (GPS-like view)
    float highTiltDeg = 0.0f;    // tilt at full height (top-down)
    float perspective = 0.6f;    // strength of far-edge shrinking
};

inline float clamp01(float v) { return std::min(1.0f, std::max(0.0f, v)); }

inline float smoothStep(float t) {
    t = clamp01(t);
    return t * t * (3.0f - 2.0f * t);
}

// 0..1 progress between start and full height.
inline float heightProgress(float y, const Settings& s) {
    float range = s.fullHeight - s.startHeight;
    if (range <= 0.0f) return y >= s.fullHeight ? 1.0f : 0.0f;
    return clamp01((y - s.startHeight) / range);
}

// Target opacity for a given altitude (and whether the player is gliding).
inline float targetOpacity(float y, bool gliding, const Settings& s) {
    float o = smoothStep(heightProgress(y, s)) * s.maxOpacity;
    if (gliding) o = std::max(o, s.glideBoost);
    return o;
}

// Move current opacity toward target, frame-rate independent.
inline float stepOpacity(float current, float target, float dtSeconds, const Settings& s) {
    float k = 1.0f - std::exp(-s.fadeSpeed * dtSeconds);
    return current + (target - current) * k;
}

// Tilt angle in degrees: flatter-looking GPS view low, top-down when high.
inline float tiltDegrees(float y, const Settings& s) {
    float t = smoothStep(heightProgress(y, s));
    return s.groundTiltDeg + (s.highTiltDeg - s.groundTiltDeg) * t;
}

// Build the 4 screen corners (TL, TR, BR, BL) of the map image.
//  center  : screen position of the map's center
//  halfSize: half the map's on-screen size in pixels
//  yawDeg  : player facing, the map rotates so "forward" points up
//  tiltDeg : 0 = flat top-down, larger = more tilted back
inline std::array<Vec2, 4> projectMapQuad(Vec2 center, float halfSize, float yawDeg,
                                          float tiltDeg, const Settings& s) {
    const float kPi = 3.14159265358979f;
    float yaw = yawDeg * kPi / 180.0f;
    float tilt = tiltDeg * kPi / 180.0f;
    float cy = std::cos(yaw), sy = std::sin(yaw);
    float ct = std::cos(tilt), st = std::sin(tilt);

    const Vec2 corners[4] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
    std::array<Vec2, 4> out{};
    for (int i = 0; i < 4; ++i) {
        // rotate by -yaw so the player's facing direction points up
        float rx = corners[i].x * cy + corners[i].y * sy;
        float ry = -corners[i].x * sy + corners[i].y * cy;
        // ry < 0 is "far" (top of screen). Far points shrink toward center.
        // depth+1 runs 0 (nearest edge) .. 2 (farthest), so the near edge
        // keeps full size and only the far side shrinks.
        float depth = -ry;                       // far = positive
        float w = 1.0f / (1.0f + s.perspective * st * (depth + 1.0f));
        out[i].x = center.x + rx * halfSize * w;
        out[i].y = center.y + ry * ct * halfSize * w;
    }
    return out;
}

}  // namespace eagleview
