#include <cassert>
#include <fstream>
#include <iostream>

#include "constants.h"
#include "match_scale.cpp"

uint8 cImage[S_SIZE][S_SIZE], cTemplate[T_SIZE][T_SIZE];

template <int H, int W> void readImage(uint8 data[H][W], std::string path) {
    std::ifstream fin(path);
    int n, m;
    fin >> n >> m;
    assert(n == H && m == W);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int v;
            fin >> v;
            data[i][j] = v;
        }
    }
}

void readData(std::string dataFolder) {
    readImage<T_SIZE, T_SIZE>(cTemplate, dataFolder + "/template.txt");
    readImage<S_SIZE, S_SIZE>(cImage, dataFolder + "/image.txt");
}

void formatPath(std::string &path) {
    assert(path.length() > 0);
    for (char &c : path) {
        if (c == '\\')
            c = '/';
    }
    if (path.back() == '/') {
        path.pop_back();
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <data-folder>\n", argv[0]);
        return 0;
    }
    std::string folderPath(argv[1]);
    formatPath(folderPath);
    readData(folderPath);
    int x, y;
    Match_also_scale(cImage, cTemplate, x, y);
    std::cout << x << ' ' << y << std::endl;
}