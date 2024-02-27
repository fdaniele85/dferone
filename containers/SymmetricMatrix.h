//
// Created by daniele on 27/02/24.
//

#pragma once

#include <cassert>
#include <cstddef>

namespace dferone::containers {

    template<class T>
    class SymmetricMatrix {
    public:
        explicit SymmetricMatrix(std::size_t rows, const T &initializer = T()) : n_(rows) {
            std::size_t size = ((n_ * n_ + n_) / 2) * sizeof(T);
            data_ = new T[size];
            for (uint i = 0; i < n_; ++i) {
                for (uint j = 0; j <= i; ++j) {
                    this->operator()(i, j) = initializer;
                }
            }
        }

        virtual ~SymmetricMatrix() {
            delete [] data_;
        }

        const T &operator()(std::size_t row, std::size_t col) const {
            assert(row < n_);
            assert(col < n_);

            return (row <= col) ? data_[row * n_ + col] : data_[col * n_ + row];
        }

        T &operator()(std::size_t row, std::size_t col) {
            assert(row < n_);
            assert(col < n_);

            return (row <= col) ? data_[row * n_ + col] : data_[col * n_ + row];
        }

    private:
        std::size_t n_;
        T* data_ {nullptr};
    };

} // namespace dferone::containers
