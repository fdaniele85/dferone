//
// Created by daniele on 27/02/24.
//

#pragma once

#include <cassert>
#include <cstddef>
#include <vector>

namespace dferone::containers {

    template<class T>
    class Matrix {
    public:
        Matrix() = default;
        Matrix(std::size_t rows, std::size_t cols, const T &initializer = T()) { reset(rows, cols, initializer); }

        void reset(std::size_t rows, std::size_t cols, const T &initializer = T()) {
            rows_ = rows;
            cols_ = cols;
            std::size_t size = rows_ * cols_;
            data_.resize(size);
            for (auto i = 0u; i < rows_; ++i) {
                for (auto j = 0u; j < cols_; ++j) {
                    this->operator()(i, j) = initializer;
                }
            }
        }

        ~Matrix() = default;

        const T &operator()(std::size_t row, std::size_t col) const {
            assert(row < rows_);
            assert(col < cols_);

            return data_[row * cols_ + col];
        }

        T &operator()(std::size_t row, std::size_t col) {
            assert(row < rows_);
            assert(col < cols_);

            return data_[cols_ * row + col];
        }

    private:
        std::size_t rows_{0};
        std::size_t cols_{0};
        std::vector<T> data_;
    };

} // namespace dferone::containers
