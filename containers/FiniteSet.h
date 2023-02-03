/*
 * FiniteSet.h
 *
 *  Created on: 11/gen/2016
 *      Author: daniele
 */

#pragma once

#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>
#include <iterator>
#include "containers.h"

namespace dferone::containers {

/// @brief La classe serve a contenere un insieme che può contenere
/// solo interi tra 0 e n (escluso)
///
/// In molte occasioni mi è capitato che gli elementi di un insieme
/// potessero essere solo gli interi tra 0 e una soglia stabilita n (escluso).
/// Però l'insieme doveva permettermi di verificare velocemente se
/// un elemento facesse o meno parte dell'insieme e mi permettesse di
/// iterare con facilità sia sugli elementi contenuti che sul complemento.

template <class T> requires std::integral<T>
class FiniteSet {
public:
	/// Il tipo degli oggetti contenuti
	using value_type 	  = T;

	/// Il tipo per gli indici
	using size_type 	  = std::size_t;

	/// Il tipo differenza tra iteratori
	using difference_type [[maybe_unused]] = int;

	/// Tipo riferimento
	using reference [[maybe_unused]] = value_type &;

	/// Tipo riferimento costante
	using const_reference [[maybe_unused]] = const value_type &;

	/// Tipo iteratore costante
	using const_iterator  = typename std::vector<value_type>::const_iterator;

	/// @name Costruttori
	/// @{

	/// @param capacity La capacità massima dell'insieme
	/// @param size La cardinalità attuale dell'insieme
	///
	/// L'insieme può contenere i valori [0, capacity); i valori
	/// [0, size) vengono effettivamente inseriti nell'insieme
	explicit inline FiniteSet(size_type capacity, size_type size = 0);
    inline FiniteSet(size_type capacity, std::initializer_list<value_type> list);
	inline FiniteSet(const FiniteSet &other) = default;
	inline FiniteSet(FiniteSet &&other) = default;
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
	inline FiniteSet &operator=(FiniteSet &&other) noexcept ;
	/// @}

private:
	/// Scambia gli elementi in posizione i e j
	inline void swappos(size_type i, size_type j) noexcept;

	/// Elementi dell'insieme
	std::vector<value_type> elements_;

	/// Posizioni degli elementi in elements_
	std::vector<size_type> positions_;

	/// Capacità massima dell'insieme
	size_type capacity_;

	/// Taglia attuale dell'insieme
	size_type size_;

};

template <class T> requires std::integral<T>
FiniteSet<T>::FiniteSet(size_type capacity, size_type size) :
		elements_(capacity), positions_(capacity), capacity_(capacity), size_(size) {
	if (size >= capacity)
		size_ = capacity;

	for (unsigned int i = 0; i < capacity; ++i) {
		elements_[i] = i;
		positions_[i] = i;
	}
}

template <class T> requires std::integral<T>
[[maybe_unused]] FiniteSet<T>::FiniteSet(size_type capacity, std::initializer_list<value_type> list) :
		FiniteSet(capacity) {
	for (auto el : list) {
		this->add(el);
	}
}


template <class T> requires std::integral<T>
typename FiniteSet<T>::size_type FiniteSet<T>::size() const noexcept {
	return size_;
}

template <class T> requires std::integral<T>
bool FiniteSet<T>::empty() const noexcept {
	return size_ == 0;
}

template <class T> requires std::integral<T>
typename FiniteSet<T>::size_type FiniteSet<T>::capacity() const noexcept {
	return capacity_;
}

template <class T> requires std::integral<T>
void FiniteSet<T>::add(value_type el) noexcept {
	if (((size_type) el) < capacity_) {
		if (!contains(el)) {
			swappos(positions_[(size_type) el], size_);
			++size_;
		}
	}
}

template <class T> requires std::integral<T>
void FiniteSet<T>::remove(value_type el) noexcept {
	if (((size_type) el) < capacity_) {
		if (contains(el)) {
			--size_;
			swappos(positions_[(size_type) el], size_);
		}
	}
}

template <class T> requires std::integral<T>
typename FiniteSet<T>::const_iterator FiniteSet<T>::remove(typename FiniteSet<T>::const_iterator it) noexcept {
	// L'iteratore dell'array non viene inficiato,
	// dato che c'è solo uno scambio di elementi: questo elemento passa
	// all'indice size_ e l'elementi in quella posizione ora si troverà
	// in questa posizione

	remove(*it);
	return it;
}

template <class T> requires std::integral<T>
bool FiniteSet<T>::contains(value_type el) const noexcept {
	return positions_[(size_type) el] < size_;
}

template <class T> requires std::integral<T>
void FiniteSet<T>::swappos(size_type i, size_type j) noexcept {
	size_type prev_i = elements_[i];
	size_type prev_j = elements_[j];

	elements_[i] = prev_j;
	elements_[j] = prev_i;

	positions_[prev_j] = i;
	positions_[prev_i] = j;
}

template <class T> requires std::integral<T>
inline std::ostream &operator<<(std::ostream &os, const FiniteSet<T> &fs) {
	return os << '{' << dferone::containers::to_string(fs) << '}';
}

template <class T> requires std::integral<T>
void FiniteSet<T>::reset() noexcept {
	size_ = 0;
}

template <class T> requires std::integral<T>
typename FiniteSet<T>::const_iterator FiniteSet<T>::cbegin() const noexcept {
	return elements_.cbegin();
}

template <class T> requires std::integral<T>
typename FiniteSet<T>::const_iterator FiniteSet<T>::cend() const noexcept {
	return elements_.cbegin() + size_;
}

template <class T> requires std::integral<T>
FiniteSet<T>::~FiniteSet() = default;

template <class T> requires std::integral<T>
inline std::ostream &operator<<(std::ostream &os, const typename FiniteSet<T>::ComplementSet &cs) {
	return os << std::to_string(cs);
}

template <class T> requires std::integral<T>
typename FiniteSet<T>::value_type FiniteSet<T>::operator[](FiniteSet::size_type pos) const noexcept {
	return elements_[pos];
}

template <class T> requires std::integral<T>
typename FiniteSet<T>::value_type FiniteSet<T>::at(typename FiniteSet<T>::size_type pos) const {
	return elements_.at(pos);
}

template <class T> requires std::integral<T>
FiniteSet<T> &FiniteSet<T>::operator=(const FiniteSet<T> &other) {
	size_ = other.size_;
	capacity_ = other.capacity_;
	positions_ = other.positions_;
	elements_ = other.elements_;
	return *this;
}

template <class T> requires std::integral<T>
FiniteSet<T> &FiniteSet<T>::operator=(FiniteSet<T> &&other) noexcept {
	size_ = other.size_;
	capacity_ = other.capacity_;
	positions_ = std::move(other.positions_);
	elements_ = std::move(other.elements_);
	return *this;
}

template <class T> requires std::integral<T>
typename FiniteSet<T>::size_type FiniteSet<T>::count(FiniteSet<T>::value_type el) const noexcept {
    if (this->contains(el)) {
        return 1;
    }
    return 0;
}

} /* namespace dferone */
