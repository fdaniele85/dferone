//
// Created by daniele on 27/02/24.
//

#pragma once

#include <cassert>
#include <cstddef>
#include <vector>

namespace dferone::containers {

    template<class T>
    class SymmetricMatrix {
    public:
        SymmetricMatrix() = default;
        explicit SymmetricMatrix(std::size_t rows, const T &initializer = T()) { reset(rows, initializer); }

        void reset(std::size_t rows, const T &initializer = T()) {
            n_ = rows;
            std::size_t size = ((n_ * n_ + n_) / 2);
            data_.resize(size);
            for (std::size_t i = 0; i < n_; ++i) {
                for (std::size_t j = 0; j <= i; ++j) {
                    this->operator[](i, j) = initializer;
                }
            }
        }

        template<typename Self>
        auto &operator[](this Self &&self, std::size_t row, std::size_t col) {
            assert(row < self.n_);
            assert(col < self.n_);

            if (row <= col) {
                return self.data_[self.idx_upper(row, col)];
            }
            return self.data_[self.idx_upper(col, row)];
        }

    private:
        std::size_t n_{0};
        std::vector<T> data_;

        [[nodiscard]] constexpr std::size_t idx_upper(std::size_t i, std::size_t j) const {
            assert(i <= j);
            return i * n_ - (i * (i - 1)) / 2 + (j - i);
        }
    };

} // namespace dferone::containers
