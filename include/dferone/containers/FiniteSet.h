/*
 * FiniteSet.h
 *
 *  Created on: 11/gen/2016
 *      Author: daniele
 */

#pragma once

#include <dferone/utils.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <vector>

namespace dferone::containers {

    /// @brief A container che puo' contenere interi in [first_element, first_element + capacity)
    ///
    /// In molte occasioni mi è capitato che gli elementi di un insieme
    /// potessero essere solo gli interi tra 0 e una soglia stabilita n (escluso).
    /// Però l'insieme doveva permettermi di verificare velocemente se
    /// un elemento facesse o meno parte dell'insieme e mi permettesse di
    /// iterare con facilità sia sugli elementi contenuti che sul complemento.

    template<class T>
        requires std::integral<T>
    class FiniteSet {
    public:
        /// Il tipo degli oggetti contenuti
        using value_type = T;

        /// Il tipo per gli indici
        using size_type = std::size_t;

        /// Il tipo differenza tra iteratori
        using difference_type [[maybe_unused]] = int;

        /// Tipo riferimento
        using reference [[maybe_unused]] = value_type &;

        /// Tipo riferimento costante
        using const_reference [[maybe_unused]] = const value_type &;

        /// Tipo iteratore costante
        using const_iterator = typename std::vector<value_type>::const_iterator;

        /// @name Costruttori
        /// @{

        /// @param capacity La capacita' dell'universo
        /// @param size La cardinalità attuale dell'insieme
        /// @param first_element Primo elemento dell'universo
        ///
        /// L'insieme puo' contenere i valori
        /// [first_element, first_element + capacity); i valori
        /// [first_element, first_element + size) vengono effettivamente inseriti.
        explicit inline FiniteSet(size_type capacity, size_type size = 0, value_type first_element = value_type{0});
        inline FiniteSet(size_type capacity, std::initializer_list<value_type> list, value_type first_element = value_type{0});
        inline FiniteSet(const FiniteSet &other) = default;
        inline FiniteSet(FiniteSet &&other) = default;

        template<std::ranges::range Range>
        inline FiniteSet(size_type capacity, Range r, value_type first_element = value_type{0}) : FiniteSet(capacity, 0, first_element) {
            for (auto el : r) {
                add(el);
            }
        }
        /// @}
        ///
        ///
        inline ~FiniteSet();

        /// @return La cardinalità attuale dell'insieme
        [[nodiscard]] inline size_type size() const noexcept;

        /// @return true se l'insieme è vuoto, false altrimenti
        [[nodiscard]] inline bool empty() const noexcept;

        /// @return La cardinalità massima dell'insieme
        [[nodiscard]] inline size_type capacity() const noexcept;

        /// @return Il primo valore dell'universo
        [[nodiscard]] inline value_type first_element() const noexcept;

        /// @param el L'elemento da inserire
        ///
        /// Aggiunge un elemento in O(1)
        inline void add(value_type el) noexcept;

        /// @param el L'elemento da rimuovere
        ///
        /// Rimuove un elemento in O(1)
        inline void remove(value_type el) noexcept;

        /// @param it Iteratore all'elemento da rimuovere
        /// @return Iteratore all'elemento successivo
        ///
        /// Rimuove un elemento in O(1)
        inline const_iterator remove(const_iterator it) noexcept;

        /// Svuota l'insieme
        inline void reset() noexcept;

        /// @param el L'elemento da controllare
        /// @return true se l'insieme contriene el, false altrimenti
        inline bool contains(value_type el) const noexcept;

        /// \param el L'elemento di cui contare le occorrenze
        /// \return 1 se l'elemente appartiene all'insieme, 0 altrimenti
        inline size_type count(value_type el) const noexcept;

        /// @name Accesso diretto
        /// @{

        /// @param pos La posizione a cui accedere
        /// @return L'elemento in posizione pos
        inline value_type operator[](size_type pos) const noexcept;
        inline value_type at(size_type pos) const;
        /// @}

        /// @param os Output stream su cui stampare
        /// @param fs Insieme da stmapare
        /// @return Output stream os
        template<class R>
        inline friend std::ostream &operator<<(std::ostream &os, const FiniteSet<R> &fs);

        /// @name Iterazione
        /// @{

        /// @return Iteratore al primo elemento
        inline const_iterator cbegin() const noexcept;
        inline const_iterator begin() const noexcept { return cbegin(); }

        /// @return Iteratore alla fine dell'insieme
        inline const_iterator cend() const noexcept;
        inline const_iterator end() const noexcept { return cend(); }
        /// @}

        /// @return L'insieme complemento
        inline auto complement() const noexcept { return std::ranges::subrange(cend(), elements_.cend()); }

        /// @name Operatori di assegnamento
        /// @{

        /// @param other Oggetto da copiare/spostare
        inline FiniteSet &operator=(const FiniteSet &other);
        inline FiniteSet &operator=(FiniteSet &&other) noexcept;
        /// @}

    private:
        /// Converte un elemento esterno in indice interno [0, capacity_)
        [[nodiscard]] inline std::optional<size_type> to_index(value_type el) const noexcept;

        /// Converte un indice interno nel valore esterno corrispondente
        [[nodiscard]] inline value_type from_index(size_type index) const noexcept;

        /// Scambia gli elementi in posizione i e j
        inline void swappos(size_type i, size_type j) noexcept;

        /// Elementi dell'insieme
        std::vector<value_type> elements_;

        /// Posizioni degli elementi in elements_
        std::vector<size_type> positions_;

        /// Capacità massima dell'insieme
        size_type capacity_;

        /// Primo valore dell'universo
        value_type first_element_;

        /// Taglia attuale dell'insieme
        size_type size_;
    };

    template<class T>
        requires std::integral<T>
    FiniteSet<T>::FiniteSet(size_type capacity, size_type size, value_type first_element)
        : elements_(capacity), positions_(capacity), capacity_(capacity), first_element_(first_element), size_(size) {
        if (size >= capacity)
            size_ = capacity;

        for (size_type i = 0; i < capacity; ++i) {
            elements_[i] = from_index(i);
            positions_[i] = i;
        }
    }

    template<class T>
        requires std::integral<T>
    [[maybe_unused]] FiniteSet<T>::FiniteSet(size_type capacity, std::initializer_list<value_type> list, value_type first_element)
        : FiniteSet(capacity, 0, first_element) {
        for (auto el : list) {
            this->add(el);
        }
    }

    template<class T>
        requires std::integral<T>
    typename FiniteSet<T>::size_type FiniteSet<T>::size() const noexcept {
        return size_;
    }

    template<class T>
        requires std::integral<T>
    bool FiniteSet<T>::empty() const noexcept {
        return size_ == 0;
    }

    template<class T>
        requires std::integral<T>
    typename FiniteSet<T>::size_type FiniteSet<T>::capacity() const noexcept {
        return capacity_;
    }

    template<class T>
        requires std::integral<T>
    typename FiniteSet<T>::value_type FiniteSet<T>::first_element() const noexcept {
        return first_element_;
    }

    template<class T>
        requires std::integral<T>
    void FiniteSet<T>::add(value_type el) noexcept {
        auto idx = to_index(el);
        if (idx && positions_[*idx] >= size_) {
            swappos(positions_[*idx], size_);
            ++size_;
        }
    }

    template<class T>
        requires std::integral<T>
    void FiniteSet<T>::remove(value_type el) noexcept {
        auto idx = to_index(el);
        if (idx && positions_[*idx] < size_) {
            --size_;
            swappos(positions_[*idx], size_);
        }
    }

    template<class T>
        requires std::integral<T>
    typename FiniteSet<T>::const_iterator FiniteSet<T>::remove(typename FiniteSet<T>::const_iterator it) noexcept {
        // L'iteratore dell'array non viene inficiato,
        // dato che c'è solo uno scambio di elementi: questo elemento passa
        // all'indice size_ e l'elementi in quella posizione ora si troverà
        // in questa posizione

        remove(*it);
        return it;
    }

    template<class T>
        requires std::integral<T>
    bool FiniteSet<T>::contains(value_type el) const noexcept {
        auto idx = to_index(el);
        return idx && positions_[*idx] < size_;
    }

    template<class T>
        requires std::integral<T>
    std::optional<typename FiniteSet<T>::size_type> FiniteSet<T>::to_index(value_type el) const noexcept {
        if (el < first_element_) {
            return std::nullopt;
        }

        auto delta = static_cast<size_type>(el - first_element_);
        if (delta >= capacity_) {
            return std::nullopt;
        }
        return delta;
    }

    template<class T>
        requires std::integral<T>
    typename FiniteSet<T>::value_type FiniteSet<T>::from_index(size_type index) const noexcept {
        return static_cast<value_type>(first_element_ + static_cast<value_type>(index));
    }

    template<class T>
        requires std::integral<T>
    void FiniteSet<T>::swappos(size_type i, size_type j) noexcept {
        value_type prev_i = elements_[i];
        value_type prev_j = elements_[j];

        elements_[i] = prev_j;
        elements_[j] = prev_i;

        positions_[*to_index(prev_j)] = i;
        positions_[*to_index(prev_i)] = j;
    }

    template<class T>
        requires std::integral<T>
    inline std::ostream &operator<<(std::ostream &os, const FiniteSet<T> &fs) {
        return os << '{' << dferone::join(fs) << '}';
    }

    template<class T>
        requires std::integral<T>
    void FiniteSet<T>::reset() noexcept {
        size_ = 0;
    }

    template<class T>
        requires std::integral<T>
    typename FiniteSet<T>::const_iterator FiniteSet<T>::cbegin() const noexcept {
        return elements_.cbegin();
    }

    template<class T>
        requires std::integral<T>
    typename FiniteSet<T>::const_iterator FiniteSet<T>::cend() const noexcept {
        return elements_.cbegin() + size_;
    }

    template<class T>
        requires std::integral<T>
    FiniteSet<T>::~FiniteSet() = default;

    template<class T>
        requires std::integral<T>
    typename FiniteSet<T>::value_type FiniteSet<T>::operator[](FiniteSet::size_type pos) const noexcept {
        return elements_[pos];
    }

    template<class T>
        requires std::integral<T>
    typename FiniteSet<T>::value_type FiniteSet<T>::at(typename FiniteSet<T>::size_type pos) const {
        return elements_.at(pos);
    }

    template<class T>
        requires std::integral<T>
    FiniteSet<T> &FiniteSet<T>::operator=(const FiniteSet<T> &other) {
        size_ = other.size_;
        capacity_ = other.capacity_;
        first_element_ = other.first_element_;
        positions_ = other.positions_;
        elements_ = other.elements_;
        return *this;
    }

    template<class T>
        requires std::integral<T>
    FiniteSet<T> &FiniteSet<T>::operator=(FiniteSet<T> &&other) noexcept {
        size_ = other.size_;
        capacity_ = other.capacity_;
        first_element_ = other.first_element_;
        positions_ = std::move(other.positions_);
        elements_ = std::move(other.elements_);
        return *this;
    }

    template<class T>
        requires std::integral<T>
    typename FiniteSet<T>::size_type FiniteSet<T>::count(FiniteSet<T>::value_type el) const noexcept {
        if (this->contains(el)) {
            return 1;
        }
        return 0;
    }

} // namespace dferone::containers
