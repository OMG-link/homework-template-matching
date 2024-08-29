#include <algorithm>
#include <climits>
#include <cmath>
#include <vector>

#include "constants.h"

namespace Utils {

// // Useless when using t-mask.
// class ItvSum2D {
//   private:
//     int height, width;
//     std::vector<std::vector<int64>> psum;

//   public:
//     ItvSum2D(std::vector<std::vector<int64>> src)
//         : height(src.size()), width(src[0].size()), psum(height + 1, std::vector<int64>(width + 1, 0ll)) {
//         for (int i = 1; i <= height; i++) {
//             for (int j = 1; j <= width; j++) {
//                 psum[i][j] = psum[i - 1][j] + psum[i][j - 1] - psum[i - 1][j - 1] + src[i - 1][j - 1];
//             }
//         }
//     }

//     // 左闭右开, 0-index
//     int64 query(int x1, int y1, int x2, int y2) { return psum[x2][y2] - psum[x2][y1] - psum[x1][y2] + psum[x1][y1]; }
// };

struct Complex {
    double real, imag;
    Complex() : Complex(0, 0) {}
    Complex(double real_, double imag_) : real(real_), imag(imag_) {}
    Complex operator+(const Complex &other) const { return Complex(real + other.real, imag + other.imag); }
    Complex operator-(const Complex &other) const { return Complex(real - other.real, imag - other.imag); }
    Complex operator*(const Complex &other) const {
        return Complex(real * other.real - imag * other.imag, real * other.imag + imag * other.real);
    }
    Complex &operator+=(const Complex &other) {
        real += other.real;
        imag += other.imag;
        return *this;
    }
    Complex &operator-=(const Complex &other) {
        real -= other.real;
        imag -= other.imag;
        return *this;
    }
    Complex &operator*=(const Complex &other) {
        double new_real = real * other.real - imag * other.imag;
        double new_imag = real * other.imag + imag * other.real;
        real = new_real;
        imag = new_imag;
        return *this;
    }
    Complex &operator/=(double other) {
        real /= other;
        imag /= other;
        return *this;
    }
};

void dft(std::vector<Complex> &a, const std::vector<int> &to, bool invert) {
    int n = a.size();

    for (int i = 0; i < n; i++) {
        if (i < to[i]) {
            std::swap(a[i], a[to[i]]);
        }
    }

    for (int len = 1; len < n; len <<= 1) {
        double ang = PI / len * (invert ? -1 : 1);
        Complex wlen(std::cos(ang), std::sin(ang));
        for (int i = 0; i < n; i += 2 * len) {
            Complex w(1, 0);
            for (int j = 0; j < len; j++) {
                Complex u = a[i + j];
                Complex v = a[i + j + len] * w;
                a[i + j] = u + v;
                a[i + j + len] = u - v;
                w *= wlen;
            }
        }
    }

    if (invert) {
        for (Complex &x : a) {
            x /= n;
        }
    }
}

std::vector<int64> fft(const std::vector<int64> &a, const std::vector<int64> &b) {
    std::vector<Complex> fa;
    int n = 1, k = 0;
    while (n < int(a.size() + b.size())) {
        n <<= 1;
        k++;
    }
    fa.resize(n);
    for (int i = 0; i < (int)a.size(); i++) {
        fa[i].real = a[i];
        fa[i].imag = b[i];
    }

    std::vector<int> to(n);
    for (int i = 0; i < n; i++) {
        to[i] = (to[i >> 1] >> 1) | ((i & 1) << (k - 1));
    }

    dft(fa, to, false);
    for (int i = 0; i < n; i++) {
        fa[i] *= fa[i];
    }
    dft(fa, to, true);

    std::vector<int64> result(n);
    for (int i = 0; i < n; i++) {
        result[i] = std::round(fa[i].imag / 2);
    }
    return result;
}

} // namespace Utils

using Utils::fft;

struct MatchResult {
    bool found;
    int64 score;
    int x, y;
};

MatchResult fastMatch(const Image &s, const Image &t, std::vector<std::vector<bool>> tMask) {
    static int call_cnt = 0;
    call_cnt++;
    const int S_HEIGHT = s.size();
    const int S_WIDTH = s[0].size();
    const int T_HEIGHT = t.size();
    const int T_WIDTH = t[0].size();
    if (T_HEIGHT > S_HEIGHT || T_WIDTH > S_WIDTH) {
        return {false, LONG_LONG_MAX, -1, -1};
    }
    std::vector<int64> arrS(S_HEIGHT * S_WIDTH, 0);
    std::vector<int64> arrT(S_HEIGHT * S_WIDTH, 0);
    std::vector<int64> arrS2(S_HEIGHT * S_WIDTH, 0);
    std::vector<int64> arrMask(S_HEIGHT * S_WIDTH, 0);
    int64 sumT2 = 0;
    for (int i = 0; i < S_HEIGHT; i++) {
        for (int j = 0; j < S_WIDTH; j++) {
            arrS[i * S_WIDTH + j] = s[i][j];
            arrS2[i * S_WIDTH + j] = static_cast<int64>(s[i][j]) * s[i][j];
        }
    }
    for (int i = 0; i < T_HEIGHT; i++) {
        for (int j = 0; j < T_WIDTH; j++) {
            arrT[i * S_WIDTH + j] = t[i][j];
            if (tMask[i][j]) {
                sumT2 += static_cast<int64>(t[i][j]) * t[i][j];
                arrMask[i * S_WIDTH + j] = 1;
            }
        }
    }
    std::reverse(arrT.begin(), arrT.end());
    std::reverse(arrMask.begin(), arrMask.end());
    auto stq = fft(arrS, arrT);
    auto s2q = fft(arrS2, arrMask);
    const int resHeight = S_HEIGHT - T_HEIGHT + 1;
    const int resWidth = S_WIDTH - T_WIDTH + 1;
    std::vector result(resHeight, std::vector<int64>(resWidth));
    for (int bx = 0; bx < resHeight; bx++) {
        for (int by = 0; by < resWidth; by++) {
            int64 s2 = s2q[bx * S_WIDTH + by + arrMask.size() - 1];
            int64 t2 = sumT2;
            int64 st = stq[bx * S_WIDTH + by + arrT.size() - 1];
            result[bx][by] = s2 - 2 * st + t2;
        }
    }
    int64 bestScore = LONG_LONG_MAX;
    int retX = -1, retY = -1;
    for (int bx = 0; bx < resHeight; bx++) {
        for (int by = 0; by < resWidth; by++) {
            int64 score = result[bx][by];
            if (score < bestScore) {
                bestScore = score;
                retX = bx;
                retY = by;
            }
        }
    }
    return {true, bestScore, retX, retY};
}