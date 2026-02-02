/*
 * BestSet.h
 *
 *  Created on: 17 giu 2016
 *      Author: daniele
 */

#ifndef DF_SORTED_VECTOR_H_
#define DF_SORTED_VECTOR_H_

#include "containers.h"
#include <functional>
#include <vector>

namespace dferone::containers {

    template<typename T, typename comparator = std::less<T>>
    class SortedVector {
    public:
        using value_type = T;
        using size_type = typename std::vector<T>::size_type;
        using reference = typename std::vector<T>::reference;
        using const_reference = typename std::vector<T>::const_reference;
        using pointer = typename std::vector<T>::pointer;
        using const_pointer = typename std::vector<T>::const_pointer;
        using const_iterator = typename std::vector<T>::const_iterator;

        /// @name (constructors)
        /// @{

        explicit SortedVector(comparator c = comparator()) : c_(c) {};

        /// @brief Copy constructor
        SortedVector(const SortedVector &other) = default;

        /// @brief Movable constructor
        SortedVector(SortedVector &&other) = default;
        /// @}

        /// \return The current size of the set
        inline size_type size() const { return elements_.size(); }

        /// @name Aggiunta elementi
        /// @{

        auto add(const value_type &element) {
            auto it = std::lower_bound(elements_.begin(), elements_.end(), element, c_);
            return elements_.insert(it, element);
        }

        auto add(value_type &&element) {
            auto it = std::lower_bound(elements_.begin(), elements_.end(), element, c_);
            return elements_.insert(it, std::move(element));
        }
        /// @}

        /// @name Accesso
        /// @{

        inline const value_type &operator[](size_type i) const { return elements_[i]; }

        inline const value_type &at(size_type i) const { return elements_.at(i); }

        inline const value_type &front() const {
            assert(!elements_.empty());
            return elements_.front();
        }

        inline const value_type &back() const {
            assert(!elements_.empty());
            return elements_.back();
        }

        inline auto erase(size_type i) { return elements_.erase(elements_.begin() + i); }
        inline auto erase(const_iterator it) { return elements_.erase(it); }
        /// @}

        /// @name Iteratori
        /// @{

        /// @brief Iteratore costante al primo elmento
        inline const_iterator cbegin() const { return std::cbegin(elements_); }

        inline const_iterator begin() const { return cbegin(); }

        /// @brief Iteratore costante alla posizione successiva all'ultimo elemento
        inline const_iterator cend() const { return std::cend(elements_); }

        inline const_iterator end() const { return cend(); }
        /// @}

        friend std::ostream &operator<<(std::ostream &out, const SortedVector<T, comparator> sorted_vector) {
            out << '[';
            join_and_print(sorted_vector, out);
            return out << ']';
        }

        /// @brief Checks if the container is empty
        /// \return True if the container is empty, false otherwise
        inline bool empty() const { return elements_.empty(); }

    private:
        /// Conserva gli elementi
        std::vector<T> elements_;

        /// Serve a paragonare gli elementi
        comparator c_;
    };

} // namespace dferone::containers

#endif