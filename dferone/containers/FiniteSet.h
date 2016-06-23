/*
 * FiniteSet.h
 *
 *  Created on: 11/gen/2016
 *      Author: daniele
 */

#ifndef FINITESET_H_
#define FINITESET_H_

#include <vector>
#include <iostream>
#include <memory>
#include <algorithm>
#include <iterator>

namespace dferone {
namespace containers {

/// @brief La classe serve a contenere un insieme che può contenere
/// solo interi tra 0 e n (escluso)
///
/// In molte occasioni mi è capitato che gli insiemi di un insieme
/// potessero essere solo gli interi tra 0 e una soglia stabilita n (escluso).
/// Però l'insieme doveva permettermi di verificare velocemente se
/// un elemento facesse o meno parte dell'insieme e mi permettesse di
/// iterare con facilità sia sugli elementi contenuti che sul complemento.
class FiniteSet {
public:
	/// Il tipo degli oggetti contenuti
	using value_type 	  = unsigned int;

	/// Il tipo per gli indici
	using size_type 	  = std::size_t;

	/// Il tipo differenza tra iteratori
	using difference_type = int;

	/// Tipo riferimento
	using reference       = value_type &;

	/// Tipo riferimento costante
	using const_reference = const value_type &;

	/// Tipo iteratore costante
	using const_iterator  = std::vector<value_type>::const_iterator;

	class ComplementSet;

	/// @name Costruttori
	/// @{

	/// @param capacity La capacità massima dell'insieme
	/// @param size La cardinalità attuale dell'insieme
	///
	/// L'insieme può contenere i valori [0, capacity); i valori
	/// [0, size) vengono effettivamente inseriti nell'insieme
	FiniteSet(size_type capacity, size_type size = 0);
	FiniteSet(size_type capacity, std::initializer_list<value_type> list);
	/// @}
	///
	///
	~FiniteSet();

	/// @return La cardinalità attuale dell'insieme
	inline size_type size()     const noexcept;

	/// @return true se l'insieme è vuoto, false altrimenti
	inline bool empty()    const noexcept;

	/// @return La cardinalità massima dell'insieme
	inline size_type capacity() const noexcept;

	/// @param el L'elemento da inserire
	///
	/// Aggiunge un elemento in O(1)
	void add(value_type el) noexcept;

	/// @param el L'elemento da rimuovere
	///
	/// Rimuove un elemento in O(1)
	void remove(value_type el) noexcept;

	/// Svuota l'insieme
	inline void reset() noexcept;

	/// @param el L'elemento da controllare
	/// @return true se l'insieme contriene el, false altrimenti
	inline bool contains(value_type el) const noexcept;

	/// @name Accesso diretto
	/// @{

	/// @param pos La posizione a cui accedere
	/// @return L'elemento in posizione pos
	value_type operator[](size_type pos) const noexcept;
	value_type at(size_type pos) const;
	/// @}

	/// @param os Output stream su cui stampare
	/// @param fs Insieme da stmapare
	/// @return Output stream os
	friend std::ostream &operator<<(std::ostream &os, const FiniteSet &fs);

	/// @name Iterazione
	/// @{

	/// @return Iteratore al primo elemento
	const_iterator cbegin() const noexcept;
	const_iterator begin() const noexcept { return cbegin(); }

	/// @return Iteratore alla fine dell'insieme
	const_iterator cend() const noexcept;
	const_iterator end() const noexcept { return cend(); }
	/// @}

	/// @return L'insieme complemento
	const ComplementSet &complement() const noexcept { return *cs_; }

private:
	/// Scambia gli elementi in posizione i e j
	void swappos(size_type i, size_type j) noexcept;

	/// Elementi dell'insieme
	std::vector<value_type> elements_;

	/// Posizioni degli elementi in elements_
	std::vector<size_type> positions_;

	/// Capacità massima dell'insieme
	size_type capacity_;

	/// Taglia attuale dell'insieme
	size_type size_;

	/// Puntatore all'insieme complemento
	std::unique_ptr<ComplementSet> cs_;
};

/// @brief Permette di iterare sull'insieme complemento
class FiniteSet::ComplementSet {
public:
	/// @name Iterazione
	/// @{

	/// @return Iteratore al primo elemento
	const_iterator begin() const noexcept {
		return fs_->end();
	}

	const_iterator cbegin() const noexcept {
		return fs_->cend();
	}

	/// @return Iteratore alla fine dell'insieme
	const_iterator end() const noexcept {
		return fs_->elements_.end();
	}

	const_iterator cend() const noexcept {
		return fs_->elements_.cend();
	}
	/// @}

	friend class FiniteSet;

	/// @param os Output stream su cui stampare
	/// @param cs Insieme complemento da stmapare
	/// @return Output stream os
	friend std::ostream &operator<<(std::ostream &os, const ComplementSet &cs);

private:
	/// Un ComplementSet può essere creato solo dal FiniteSet associato
	///
	/// @param fs @param fs L'insieme di cui si è complemento
	ComplementSet(FiniteSet *fs) : fs_(fs) {}

	/// Puntatore all'insieme associato
	FiniteSet *fs_;
};

}
}

/// @namespace std
/// @brief Fa l'overload di alcune funzioni presenti in std
namespace std {
/// @fn std::string to_string(const dferone::containers::FiniteSet &fs)
/// Fa l'overload di std::to_string per produrre una stringa che rappresenta il FiniteSet
///
/// @param fs Insieme da stampare
/// @return La stringa che lo rappresenta
std::string to_string(const dferone::containers::FiniteSet &fs) {
	string s("{ ");
	auto it = cbegin(fs), end = cend(fs);

	if (it != end) {
		s += to_string(*it);

		while (++it != end) {
			s += ", " + to_string(*it);
		}
	}

	return s + " }";
}

/// Fa l'overload di std::to_string per produrre una stringa che rappresenta il ComplementSet
///
/// @param cs Insieme complemento da stampare
/// @return La stringa che lo rappresenta
std::string to_string(const dferone::containers::FiniteSet::ComplementSet &cs) {
	string s("{ ");
	auto it = cbegin(cs), end = cend(cs);

	if (it != end) {
		s += to_string(*it);

		while (++it != end) {
			s += ", " + to_string(*it);
		}
	}

	return s + " }";
}

}

namespace dferone {
namespace containers {


FiniteSet::FiniteSet(size_type capacity, size_type size) :
		elements_(capacity), positions_(capacity), capacity_(capacity), size_(size), cs_(new ComplementSet(this)) {
	if (size >= capacity)
		size_ = capacity;

	for (unsigned int i = 0; i < capacity; ++i) {
		elements_[i] = i;
		positions_[i] = i;
	}
}

FiniteSet::FiniteSet(size_type capacity, std::initializer_list<value_type> list) :
		FiniteSet(capacity) {
	for (auto el : list) {
		this->add(el);
	}
}

FiniteSet::size_type FiniteSet::size() const noexcept {
	return size_;
}

bool FiniteSet::empty() const noexcept {
	return size_ == 0;
}

FiniteSet::size_type FiniteSet::capacity() const noexcept {
	return capacity_;
}

void FiniteSet::add(value_type el) noexcept {
	if (el < capacity_) {
		if (!contains(el)) {
			swappos(positions_[el], size_);
			++size_;
		}
	}
}

void FiniteSet::remove(value_type el) noexcept {
	if (el < capacity_) {
		if (contains(el)) {
			--size_;
			swappos(positions_[el], size_);
		}
	}
}

bool FiniteSet::contains(value_type el) const noexcept {
	return positions_[el] < size_;
}

void FiniteSet::swappos(size_type i, size_type j) noexcept {
	size_type prev_i = elements_[i];
	size_type prev_j = elements_[j];

	elements_[i] = prev_j;
	elements_[j] = prev_i;

	positions_[prev_j] = i;
	positions_[prev_i] = j;
}

std::ostream &operator<<(std::ostream &os, const FiniteSet &fs) {
	return os << std::to_string(fs);
}

void FiniteSet::reset() noexcept {
	size_ = 0;
}

FiniteSet::const_iterator FiniteSet::cbegin() const noexcept {
	return elements_.cbegin();
}

FiniteSet::const_iterator FiniteSet::cend() const noexcept {
	return elements_.cbegin() + size_;
}

FiniteSet::~FiniteSet() = default;

std::ostream &operator<<(std::ostream &os, const FiniteSet::ComplementSet &cs) {
	return os << std::to_string(cs);
}

FiniteSet::value_type FiniteSet::operator[](FiniteSet::size_type pos) const noexcept {
	return elements_[pos];
}

FiniteSet::value_type FiniteSet::at(FiniteSet::size_type pos) const {
	return elements_.at(pos);
}


} /* namespace finiteset */
} /* namespace dferone */




#endif /* FINITESET_H_ */
