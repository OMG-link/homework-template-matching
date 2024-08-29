#include <cmath>
#include <vector>

#include "constants.h"
#include "fast_match.cpp"

const int64 SCORE_THRESHOLD = (int64)(256 * 256 / 3) * (T_SIZE * T_SIZE) / 16 * DETECT_SENSITIVITY;

namespace ImageUtil {

uint8 bilinearInterpolation(const Image &image, float x, float y) {
    int x1 = std::floor(x);
    int y1 = std::floor(y);
    int x2 = std::ceil(x);
    int y2 = std::ceil(y);

    float a = x - x1;
    float b = y - y1;

    return static_cast<uint8>((1 - a) * (1 - b) * image[y1][x1] + a * (1 - b) * image[y1][x2] +
                              (1 - a) * b * image[y2][x1] + a * b * image[y2][x2]);
}

void rotateImage(const Image &originalImage, float rad, Image &resultImage,
                 std::vector<std::vector<bool>> &resultMask) {
    int originalHeight = originalImage.size();
    int originalWidth = originalImage[0].size();
    int canvasLength = 2 * std::max(originalHeight, originalWidth);

    // 计算旋转后图像的大小，并初始化result和mask
    Image rotatedImage(canvasLength, std::vector<uint8>(canvasLength, 0));
    std::vector<std::vector<bool>> rotatedMask(canvasLength, std::vector<bool>(canvasLength, false));

    float cosRad = cos(rad);
    float sinRad = sin(rad);
    float offsetX = (canvasLength - originalHeight) / 2.0;
    float offsetY = (canvasLength - originalWidth) / 2.0;

    // 旋转图像
    for (int x = 0; x < canvasLength; x++) {
        for (int y = 0; y < canvasLength; y++) {
            // 反向映射坐标
            int originalX =
                -(x - canvasLength / 2) * sinRad + (y - canvasLength / 2) * cosRad + canvasLength / 2 - offsetX;
            int originalY =
                (x - canvasLength / 2) * cosRad + (y - canvasLength / 2) * sinRad + canvasLength / 2 - offsetY;

            if (originalY >= 0 && originalY < originalWidth && originalX >= 0 && originalX < originalHeight) {
                rotatedImage[y][x] = bilinearInterpolation(originalImage, originalY, originalX);
                rotatedMask[y][x] = true;
            }
        }
    }

    // 裁剪透明边界
    int minX = canvasLength, minY = canvasLength, maxX = 0, maxY = 0;
    for (int x = 0; x < canvasLength; x++) {
        for (int y = 0; y < canvasLength; y++) {
            if (rotatedMask[x][y]) {
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
            }
        }
    }

    // 裁剪结果
    int resultHeight = maxX - minX + 1;
    int resultWidth = maxY - minY + 1;

    resultImage = Image(resultHeight, std::vector<uint8>(resultWidth, 0));
    resultMask = std::vector<std::vector<bool>>(resultHeight, std::vector<bool>(resultWidth, false));

    for (int x = 0; x < resultHeight; x++) {
        for (int y = 0; y < resultWidth; y++) {
            resultImage[x][y] = rotatedImage[minX + x][minY + y];
            resultMask[x][y] = rotatedMask[minX + x][minY + y];
        }
    }
}

} // namespace ImageUtil

using ImageUtil::rotateImage;

MatchResult testRad(const Image &vs, const Image &vt, float rad) {
    while (rad < 0) {
        rad += 2 * PI;
    }
    while (rad > 2 * PI) {
        rad -= 2 * PI;
    }
    Image rotatedT;
    std::vector<std::vector<bool>> tMask;
    rotateImage(vt, rad, rotatedT, tMask);
    auto result = fastMatch(vs, rotatedT, tMask);
    // 获取左上角坐标对应的位置
    int rotatedHeight = rotatedT.size();
    int rotatedWidth = rotatedT[0].size();
    if (rad < 0.5 * PI) {
        for (int y = 0; y < rotatedWidth; y++) {
            if (tMask[0][y]) {
                result.y += y;
                break;
            }
        }
    } else if (rad < PI) {
        for (int x = 0; x < rotatedHeight; x++) {
            if (tMask[x][rotatedWidth - 1]) {
                result.x += x;
                result.y += rotatedWidth - 1;
                break;
            }
        }
    } else if (rad < 1.5 * PI) {
        for (int y = 0; y < rotatedWidth; y++) {
            if (tMask[rotatedHeight - 1][y]) {
                result.x += rotatedHeight - 1;
                result.y += y;
                break;
            }
        }
    } else {
        for (int x = 0; x < rotatedHeight; x++) {
            if (tMask[x][0]) {
                result.x += x;
                break;
            }
        }
    }
    return result;
}

std::tuple<int, int, int, int> getSubImageRoot(int x, int y, int tHeight, int tWidth, float rad) {
    float d = std::atan2(static_cast<float>(tHeight), static_cast<float>(tWidth)) + rad;
    float dlen = std::sqrt(static_cast<float>(tHeight * tHeight + tWidth * tWidth)) / 2;
    float blen = std::max(tHeight, tWidth);
    float bx = x + dlen * std::sin(d) - blen;
    float by = y + dlen * std::cos(d) - blen;
    return {bx, by, bx + 2 * blen, by + 2 * blen};
}

Image getSubImage(const Image &originalImage, int lx, int ly, int rx, int ry) {
    int height = rx - lx;
    int width = ry - ly;
    Image resultImage(height, std::vector<uint8>(width));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            resultImage[i][j] = originalImage[lx + i][ly + j];
        }
    }
    return resultImage;
}

std::pair<float, MatchResult> findValley(const Image &vs, const Image &vt, float lrad, float rrad) {
    const int TP_LIMIT = 10;
    const float phi = (std::sqrt(5.0) - 1.0) / 2.0;
    float x1 = rrad - phi * (rrad - lrad);
    float x2 = lrad + phi * (rrad - lrad);

    MatchResult result1 = testRad(vs, vt, x1);
    MatchResult result2 = testRad(vs, vt, x2);

    MatchResult bestResult = result1.score < result2.score ? result1 : result2;
    float bestRad = result1.score < result2.score ? x1 : x2;

    for (int i = 0; i < TP_LIMIT; ++i) {
        if (result1.score < result2.score) {
            rrad = x2;
            x2 = x1;
            result2 = result1;
            x1 = rrad - phi * (rrad - lrad);
            result1 = testRad(vs, vt, x1);
        } else {
            lrad = x1;
            x1 = x2;
            result1 = result2;
            x2 = lrad + phi * (rrad - lrad);
            result2 = testRad(vs, vt, x2);
        }

        if (result1.score < result2.score) {
            bestResult = result1;
            bestRad = x1;
        } else {
            bestResult = result2;
            bestRad = x2;
        }
    }

    return {bestRad, bestResult};
}

float Match_also_orient(uint8 s[S_SIZE][S_SIZE], uint8 t[T_SIZE][T_SIZE], int &retX, int &retY) {
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
    const int STEP_NUM = 16;
    auto getRad = [&](int id) -> float { return 2 * PI * id / STEP_NUM; };
    std::vector<MatchResult> basicResult;
    for (int i = 0; i < STEP_NUM; i++) {
        float rad = getRad(i);
        auto result = testRad(vs, vt, rad);
        basicResult.push_back(result);
        // auto [lx, ly, rx, ry] = getSubImageRoot(basicResult[i].x, basicResult[i].y, T_SIZE, T_SIZE, getRad(i));
        // fprintf(stderr, "rad=%f, score=%lld, box=[(%d,%d),(%d,%d)]\n", rad, result.score, lx, ly, rx, ry);
    }
    // Search around valleys
    const int MAX_SEARCH_NUM = 2;
    std::vector<int> valleys;
    for (int i = 0; i < STEP_NUM; i++) {
        int64 lastScore = basicResult[(i + STEP_NUM - 1) % STEP_NUM].score;
        int64 nextScore = basicResult[(i + 1) % STEP_NUM].score;
        int64 currentScore = basicResult[i].score;
        if (currentScore < lastScore && currentScore < nextScore) {
            valleys.push_back(i);
        }
    }
    std::sort(valleys.begin(), valleys.end(),
              [&](int x, int y) -> bool { return basicResult[x].score < basicResult[y].score; });
    int64 bestScore = LONG_LONG_MAX;
    float bestRad = 0;
    for (int i = 0; i < (int)valleys.size() && i < MAX_SEARCH_NUM; i++) {
        int valleyId = valleys[i];
        auto [lx, ly, rx, ry] =
            getSubImageRoot(basicResult[valleyId].x, basicResult[valleyId].y, T_SIZE, T_SIZE, getRad(valleyId));
        lx = std::max(lx, 0);
        ly = std::max(ly, 0);
        rx = std::min(rx, S_SIZE);
        ry = std::min(ry, S_SIZE);
        auto subvs = getSubImage(vs, lx, ly, rx, ry);
        auto [resultRad, result] = findValley(subvs, vt, getRad(valleyId - 1), getRad(valleyId + 1));
        result.x += lx;
        result.y += ly;
        if (result.score < bestScore) {
            bestScore = result.score;
            bestRad = resultRad;
            retX = result.x;
            retY = result.y;
        }
    }
    fprintf(stderr, "Score=%lld, Rad=%f, X=%d, Y=%d\n", bestScore, bestRad, retX, retY);
    return bestRad;
}