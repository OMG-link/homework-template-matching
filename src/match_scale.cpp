#include <cmath>
#include <vector>

#include "constants.h"
#include "fast_match.cpp"

const int64 SCORE_THRESHOLD = (int64)(256 * 256 / 3) * (T_SIZE * T_SIZE) / 16 * DETECT_SENSITIVITY;

namespace ImageUtil {

void scaleImage(const Image &originalImage, float scale, Image &resultImage) {
    int originalHeight = originalImage.size();
    int originalWidth = originalImage[0].size();

    int newHeight = static_cast<int>(originalHeight * scale);
    int newWidth = static_cast<int>(originalWidth * scale);
    resultImage.assign(newHeight, std::vector<uint8>(newWidth));

    for (int i = 0; i < newHeight; ++i) {
        for (int j = 0; j < newWidth; ++j) {
            int orig_i = static_cast<int>(i / scale);
            int orig_j = static_cast<int>(j / scale);

            orig_i = std::min(orig_i, originalHeight - 1);
            orig_j = std::min(orig_j, originalWidth - 1);

            resultImage[i][j] = originalImage[orig_i][orig_j];
        }
    }
}

} // namespace ImageUtil

using ImageUtil::scaleImage;

MatchResult testScale(const Image &vs, const Image &vt, float scale) {
    Image scaledT;
    scaleImage(vt, scale, scaledT);
    std::vector tMask(scaledT.size(), std::vector<bool>(scaledT[0].size(), true));
    auto result = fastMatch(vs, scaledT, tMask);
    // 归一化
    result.score = result.score * (vt.size() * vt[0].size()) / (scaledT.size() * scaledT[0].size());
    return result;
}

std::pair<float, MatchResult> findValley(const Image &vs, const Image &vt, float lsr, float rsr) {
    const int TP_LIMIT = 10;
    const float phi = (std::sqrt(5.0) - 1.0) / 2.0;
    float x1 = rsr - phi * (rsr - lsr);
    float x2 = lsr + phi * (rsr - lsr);

    MatchResult result1 = testScale(vs, vt, x1);
    MatchResult result2 = testScale(vs, vt, x2);

    MatchResult bestResult = result1.score < result2.score ? result1 : result2;
    float bestScale = result1.score < result2.score ? x1 : x2;

    for (int i = 0; i < TP_LIMIT; ++i) {
        if (result1.score < result2.score) {
            rsr = x2;
            x2 = x1;
            result2 = result1;
            x1 = rsr - phi * (rsr - lsr);
            result1 = testScale(vs, vt, x1);
        } else {
            lsr = x1;
            x1 = x2;
            result1 = result2;
            x2 = lsr + phi * (rsr - lsr);
            result2 = testScale(vs, vt, x2);
        }

        if (result1.score < result2.score) {
            bestResult = result1;
            bestScale = x1;
        } else {
            bestResult = result2;
            bestScale = x2;
        }
    }

    return {bestScale, bestResult};
}

float Match_also_scale(uint8 s[S_SIZE][S_SIZE], uint8 t[T_SIZE][T_SIZE], int &retX, int &retY) {
    Image vs(S_SIZE, std::vector<uint8>(S_SIZE, 0));
    Image vt(T_SIZE, std::vector<uint8>(T_SIZE, 0));
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
    // Do basic search
    const int STEP_NUM = 8;
    const float MAX_SCALE = (float)S_SIZE / T_SIZE;
    const float MIN_SCALE = (float)16 / T_SIZE;
    auto getScale = [&](int id) -> float {
        return MIN_SCALE * pow(MAX_SCALE / MIN_SCALE, static_cast<float>(id) / (STEP_NUM - 1));
    };
    std::vector<int64> basicScores;
    for (int i = 0; i < STEP_NUM; i++) {
        auto result = testScale(vs, vt, getScale(i));
        if (result.found) {
            basicScores.push_back(result.score);
            fprintf(stderr, "scale=%f, score=%lld\n", getScale(i), result.score);
        } else {
            basicScores.push_back(LONG_LONG_MAX);
        }
    }
    // Search around valleys
    const int MAX_SEARCH_NUM = 2;
    std::vector<int> valleys;
    for (int i = 1; i < STEP_NUM - 1; i++) {
        int64 lastScore = basicScores[i - 1];
        int64 nextScore = basicScores[i + 1];
        int64 currentScore = basicScores[i];
        if ((i == 1 || currentScore < lastScore) && (i == STEP_NUM - 2 || currentScore < nextScore)) {
            valleys.push_back(i);
        }
    }
    std::sort(valleys.begin(), valleys.end(), [&](int x, int y) -> bool { return basicScores[x] < basicScores[y]; });
    int64 bestScore = LONG_LONG_MAX;
    float bestScale = 0;
    for (int i = 0; i < (int)valleys.size() && i < MAX_SEARCH_NUM; i++) {
        int valleyId = valleys[i];
        auto [resultScale, result] = findValley(vs, vt, getScale(valleyId - 1), getScale(valleyId + 1));
        if (result.score < bestScore) {
            bestScore = result.score;
            bestScale = resultScale;
            retX = result.x;
            retY = result.y;
        }
    }
    fprintf(stderr, "Score=%lld, Scale=%f, X=%d, Y=%d\n", bestScore, bestScale, retX, retY);
    return bestScale;
}