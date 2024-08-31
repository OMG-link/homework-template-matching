#include <vector>

#include "constants.h"
#include "fast_match.cpp"

bool Match_accelerated(uint8 s[S_SIZE][S_SIZE], uint8 t[T_SIZE][T_SIZE], int &retX, int &retY) {
    Image vs(S_SIZE, S_SIZE);
    Image vt(T_SIZE, T_SIZE);
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
    fprintf(stderr, "Score=%f\n", result.score);
    if (result.score > 0.9) {
        retX = result.x;
        retY = result.y;
        return true;
    } else {
        return false;
    }
}