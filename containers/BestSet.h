/*
 * BestSet.h
 *
 *  Created on: 17 giu 2016
 *      Author: daniele
 */

#ifndef BESTSET_H_
#define BESTSET_H_

#include <vector>
#include <functional>

namespace dferone {
namespace containers {

/// \brief Insieme di dimensione fissa che conserva solo i migliori elementi
///
/// \tparam T Tipo di elementi da conservare
/// \tparam comparator Comparatore di elementi, deve implementare "bool operator(const T& lhs, const T& rhs)",
/// che restituisce vero se lhs è da conservare rispetto a rhs
///
template <typename T, typename comparator = std::greater<T>>
class BestSet {
public:
	/// Il tipo degli elementi conservati
	using value_type = T;

	/// Il size type per gli indici
	using size_type = typename std::vector<T>::size_type;

	/// Tipo riferimento al tipo degli elementi conservati
	using reference = typename std::vector<T>::reference;

	/// Tipo riferimento costante al tipo degli elementi conservati
	using const_reference = typename std::vector<T>::const_reference;

	/// Tipo puntatore al tipo degli elementi conservati
	using pointer = typename std::vector<T>::pointer;

	/// Tipo puntatore costante al tipo degli elementi conservati
	using const_pointer = typename std::vector<T>::const_pointer;

	/// Iteratore costante dell'insieme
	using const_iterator = typename std::vector<T>::const_iterator;

	/// @name Costruttori
	/// @{

	/// @param capacity La capacità dell'insieme
	inline BestSet(size_type capacity, comparator c = comparator()) : capacity_(capacity) , elements_(capacity), c_(c) {};

	/// @brief Costruttore di copia
	BestSet(const BestSet &other) = default;

	/// @brief Costruttore di spostamento
	BestSet(BestSet &&other) = default;
	/// @}

	inline size_type size() const {
		return size_;
	}

	/// @name Aggiunta elementi
	/// @{

	/// @brief Aggiunge un elemento all'insieme
	///
	/// Il metodo prova ad aggiungere un elemento all'insieme.
	/// L'aggiunta non avviene sempre: se l'insieme ha raggiunto
	/// la capacità, l'elemento viene inserito solo se è migliore
	/// di almeno uno già presente.
	///
	/// @param element L'elemento da inserire
	/// @return true se l'elemento è stato inserito, false altrimenti
	inline bool add(const value_type &element) {
		// Se la size è minore della capacità l'elemento viene inserito in fondo
		if (size_ < capacity_) {
			elements_[size_] = element;
			++size_;
		}

		// Altrimenti, se l'elemento è migliore dell'ultimo elemento dell'insieme,
		// allora viene inserito al posto di quest'ultimo
		else if (c_(element, elements_[size_ - 1])) {
			elements_[size_ - 1] = element;
		}

		// L'elemento non è stato inserito
		else return false;

		// Riordina l'array
		sort();
		return true;
	}

	inline bool add(value_type &&element) {
		// Se la size è minore della capacità l'elemento viene inserito in fondo
		if (size_ < capacity_) {
			elements_[size_] = std::move(element);
			++size_;
		}

		// Altrimenti, se l'elemento è migliore dell'ultimo elemento dell'insieme,
		// allora viene inserito al posto di quest'ultimo
		else if (c_(element, elements_[size_ - 1])) {
			elements_[size_ - 1] = std::move(element);
		}

		// L'elemento non è stato inserito
		else return false;

		// Riordina l'array
		sort();
		return true;
	}
	/// @}

	/// @name Accesso
	/// @{

	/// @brief Accede ad un elemento dell'insieme
	/// @param i Posizione dell'emento a cui accedere
	/// @return L'elemento in posizione i
	inline const value_type &operator[](size_type i) const {
		return elements_[i];
	}

	inline const value_type &at(size_type i) const {
		return elements_.at(i);
	}

	/// @brief Accede al miglior elemento dell'insieme
	inline const value_type &top() const {
		return elements_[0];
	}

	/// @brief Estrae un elemento dall'insieme
	/// @param i Indice dell'insieme da estrarre
	/// @return L'lemento estratto
	inline value_type pop(size_type i = 0) {

		// Sposta l'elemento in una variabile d'appoggio
		value_type ret(std::move(elements_[i]));

		// Shifta gli altri elementi
		while (i < size_ - 1) {
			elements_[i] = std::move(elements_[i+1]);
			++i;
		}

		// Diminuisce la taglia dell'insieme
		--size_;

		return ret;
	}
	/// @}

	/// @name Iteratori
	/// @{

	/// @brief Iteratore costante al primo elmento
	inline const_iterator cbegin() const {
		return std::cbegin(elements_);
	}

	inline const_iterator begin() const {
		return cbegin();
	}

	/// @brief Iteratore costante alla posizione successiva all'ultimo elemento
	inline const_iterator cend() const {
		return cbegin() + size_;
	}

	inline const_iterator end() const {
		return cend();
	}
	/// @}


private:
	/// Capacità massima dell'insieme
	size_type capacity_;

	/// Numeri di elementi attualmente nell'insieme
	size_type size_ { 0 };

	/// Conserva gli elementi
	std::vector<T> elements_;

	/// Serve a paragonare gli elementi
	comparator c_;

	/// Riordina gli elemnti dopo un inserimento
	inline void sort() {
		size_type s = size_ - 1;

		while (s > 0) {
			if (c_(elements_[s], elements_[s-1]))
				std::swap(elements_[s], elements_[s-1]);
			else
				break;

			--s;
		}
	}
};

} /* namespace containers */
} /* namespace fdaniele */

#endif /* BESTSET_H_ */
