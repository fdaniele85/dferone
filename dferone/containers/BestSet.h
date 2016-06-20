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

/**
 * \tparam T Si sI
 * \tparam comparator brav
 */
template <typename T, typename comparator = std::greater<T>>
class BestSet {
public:
	using value_type = T;
	using size_type = typename std::vector<T>::size_type;
	using reference = typename std::vector<T>::reference;
	using const_reference = typename std::vector<T>::const_reference;
	using pointer = typename std::vector<T>::pointer;
	using const_pointer = typename std::vector<T>::const_pointer;
	using const_iterator = typename std::vector<T>::const_iterator;

	BestSet(size_type capacity) : capacity_(capacity) , elements_(capacity) {};

	BestSet(const BestSet &other) = default;
	BestSet(BestSet &&other) = default;

	~BestSet() {};

	size_type size() const {
		return size_;
	}

	bool add(const value_type &element) {
		comparator c;

		if (size_ < capacity_) {
			elements_[size_] = element;
			++size_;
		}
		else if (c(element, elements_[size_ - 1])) {
			elements_[size_ - 1] = element;
		}
		else return false;

		sort();
		return true;
	}

	bool add(value_type &&element) {
		comparator c;

		if (size_ < capacity_) {
			elements_[size_] = std::move(element);
			++size_;
		}
		else if (c(element, elements_[size_ - 1])) {
			elements_[size_ - 1] = std::move(element);
		}
		else return false;

		sort();
		return true;
	}

	value_type &&operator[](size_type i) {
		return std::move(elements_[i]);
	}

	const value_type &operator[](size_type i) const {
		return elements_[i];
	}

	const value_type &at(size_type i) const {
		return elements_.at(i);
	}

	const_iterator begin() const {
		return std::begin(elements_);
	}

	const_iterator end() const {
		return begin() + size_;
	}

private:
	size_type capacity_;
	size_type size_ { 0 };

	std::vector<T> elements_;

	void sort() {
		comparator c;
		size_type s = size_ - 1;
		while (s > 0) {
			if (c(elements_[s], elements_[s-1]))
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
