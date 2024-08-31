#ifndef _IMAGE_HPP
#define _IMAGE_HPP

#include <stdexcept>
#include <vector>

#include "constants.h"

class Image {
  public:
    // Non-const access to a row using an integer index
    class Row {
      public:
        Row(uint8 *rowPtr, int width) : rowPtr(rowPtr), width(width) {}

        uint8 &operator[](int col) {
            if (col < 0 || col >= width) {
                throw std::out_of_range("Index out of bounds");
            }
            return rowPtr[col];
        }

      private:
        uint8 *rowPtr;
        int width;
    };

    // Const access to a row using an integer index
    class ConstRow {
      public:
        ConstRow(const uint8 *rowPtr, int width) : rowPtr(rowPtr), width(width) {}

        const uint8 &operator[](int col) const {
            if (col < 0 || col >= width) {
                throw std::out_of_range("Index out of bounds");
            }
            return rowPtr[col];
        }

      private:
        const uint8 *rowPtr;
        int width;
    };

    int height, width;

    // Constructor
    Image() : Image(0, 0) {}

    // Constructor
    Image(int height, int width) : height(height), width(width), data(height * width) {}

    // Non-const access to an individual pixel using a (row, col) pair
    uint8 &operator[](std::pair<int, int> position) {
        int row = position.first;
        int col = position.second;
        if (row < 0 || row >= height || col < 0 || col >= width) {
            throw std::out_of_range("Index out of bounds");
        }
        return data[row * width + col];
    }

    // Const access to an individual pixel using a (row, col) pair
    const uint8 &operator[](std::pair<int, int> position) const {
        int row = position.first;
        int col = position.second;
        if (row < 0 || row >= height || col < 0 || col >= width) {
            throw std::out_of_range("Index out of bounds");
        }
        return data[row * width + col];
    }

    Row operator[](int rowIndex) {
        if (rowIndex < 0 || rowIndex >= height) {
            throw std::out_of_range("Index out of bounds");
        }
        return Row(data.data() + rowIndex * width, width);
    }

    ConstRow operator[](int rowIndex) const {
        if (rowIndex < 0 || rowIndex >= height) {
            throw std::out_of_range("Index out of bounds");
        }
        return ConstRow(data.data() + rowIndex * width, width);
    }

  private:
    std::vector<uint8> data;
};

#endif