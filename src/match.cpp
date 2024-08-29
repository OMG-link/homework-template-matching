#include <climits>

#include "constants.h"

const int64 SCORE_THRESHOLD = (int64)(256 * 256 / 3) * (T_SIZE * T_SIZE) / 16 * DETECT_SENSITIVITY;

int scoreFunc(int sPixel, int tPixel) {
    int delta = sPixel - tPixel;
    return delta * delta;
}

bool Match(uint8 s[S_SIZE][S_SIZE], uint8 t[T_SIZE][T_SIZE], int &retX, int &retY) {
    int bestScore = INT_MAX;
    for (int bx = 0; bx <= S_SIZE - T_SIZE; bx++) {
        for (int by = 0; by <= S_SIZE - T_SIZE; by++) {
            int score = 0;
            for (int dx = 0; dx < T_SIZE; dx++) {
                for (int dy = 0; dy < T_SIZE; dy++) {
                    int sx = bx + dx;
                    int sy = by + dy;
                    int tx = dx;
                    int ty = dy;
                    score += scoreFunc(s[sx][sy], t[tx][ty]);
                }
            }
            if (score < bestScore) {
                bestScore = score;
                retX = bx;
                retY = by;
            }
        }
    }
    if (bestScore < SCORE_THRESHOLD) {
        return true;
    } else {
        return false;
    }
}