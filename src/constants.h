#ifndef __CONSTANTS_H
#define __CONSTANTS_H

#include <cmath>
#include <vector>

using uint8 = unsigned char;
using int64 = long long;

using Image = std::vector<std::vector<uint8>>;

const double PI = acos(-1);

const int S_SIZE = 256;
const int T_SIZE = 64;
const float DETECT_SENSITIVITY = 1.0;

#endif