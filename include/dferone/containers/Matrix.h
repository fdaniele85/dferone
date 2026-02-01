//
// Created by daniele on 27/02/24.
//

#pragma once

#include <cassert>
#include <cstddef>

namespace dferone::containers {

    template<class T>
    class Matrix {
    public:
        Matrix() = default;
        Matrix(std::size_t rows, std::size_t cols, const T &initializer = T()) {
            reset(rows, cols, initializer);
        }

        void reset(std::size_t rows, std::size_t cols, const T &initializer = T()) {
            rows_ = rows;
            cols_ = cols;
            std::size_t size = rows_ * cols_ * sizeof(T);
            data_ = new T[size];
            for (uint i = 0; i < rows_; ++i) {
                for (uint j = 0; j < cols_; ++j) {
                    this->operator()(i, j) = initializer;
                }
            }
        }

        virtual ~Matrix() {
            delete [] data_;
        }

        const T &operator()(std::size_t row, std::size_t col) const {
            assert(row < rows_);
            assert(col < cols_);

            return data_[row][col];
        }

        T &operator()(std::size_t row, std::size_t col) {
            assert(row < rows_);
            assert(col < cols_);

            return data_[cols_ * row + col];
        }



    private:
        std::size_t rows_ {0};
        std::size_t cols_ {0};
        T* data_ {nullptr};
    };

} // namespace dferone::containers
