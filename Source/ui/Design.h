
#pragma once
namespace UI {
    inline float scale = 1.0f;
    constexpr int dp   = 8;
    constexpr int gap  = 2 * dp;
    constexpr int pad  = 3 * dp;
    constexpr int rowH = 9 * dp;
    constexpr int minHit = 44;
    inline int u(int n) { return int(n * dp * scale); }
}
