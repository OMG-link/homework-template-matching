#include <vector>

#include "constants.h"
#include "fast_match.cpp"

const int64 SCORE_THRESHOLD = (int64)(256 * 256 / 3) * (T_SIZE * T_SIZE) / 16 * DETECT_SENSITIVITY;

bool Match_accelerated(uint8 s[S_SIZE][S_SIZE], uint8 t[T_SIZE][T_SIZE], int &retX, int &retY) {
    std::vector vs(S_SIZE, std::vector<uint8>(S_SIZE, 0));
    std::vector vt(T_SIZE, std::vector<uint8>(T_SIZE, 0));
    std::vector tMask(T_SIZE, std::vector<bool>(T_SIZE, true));
    for (int i = 0; i < S_SIZE; i++) {
        for (int j = 0; j < S_SIZE; j++) {
            vs[i][j] = s[i][j];
        }
    }
    for (int i = 0; i < T_SIZE; i++) {
        for (int j = 0; j < T_SIZE; j++) {
            vt[i][j] = t[i][j];
        }
    }
    auto result = fastMatch(vs, vt, tMask);
    if (result.found && result.score < SCORE_THRESHOLD) {
        retX = result.x;
        retY = result.y;
        return true;
    } else {
        return false;
    }
}