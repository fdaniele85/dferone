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

class FiniteSet {
public:
	using value_type 	  = unsigned int;
	using size_type 	  = std::size_t;
	using difference_type = int;
	using reference       = value_type &;
	using const_reference = const value_type &;
	using const_iterator  = std::vector<value_type>::const_iterator;

	class ComplementSet;

	FiniteSet(size_type capacity, size_type size = 0);
	FiniteSet(size_type capacity, std::initializer_list<value_type> list);
	~FiniteSet();

	size_type size()     const noexcept;
	bool      empty()    const noexcept;
	size_type capacity() const noexcept;

	void add(value_type el) noexcept;
	void remove(value_type el) noexcept;
	void reset() noexcept;
	bool contains(value_type el) const noexcept;

	value_type operator[](size_type pos) const noexcept;
	value_type at(size_type pos) const;

	friend std::ostream &operator<<(std::ostream &os, const FiniteSet &fs);

	const_iterator cbegin() const noexcept;
	const_iterator cend() const noexcept;

	const_iterator begin() const noexcept { return cbegin(); }
	const_iterator end() const noexcept { return cend(); }

	const ComplementSet &complement() const noexcept { return *cs_; }

private:
	void swappos(size_type i, size_type j) noexcept;

	std::vector<value_type> elements_;
	std::vector<size_type> positions_;

	size_type capacity_;
	size_type size_;

	std::unique_ptr<ComplementSet> cs_;
};

class FiniteSet::ComplementSet {
public:
	const_iterator begin() const noexcept {
		return fs_->end();
	}

	const_iterator end() const noexcept {
		return fs_->elements_.end();
	}
	friend class FiniteSet;

	friend std::ostream &operator<<(std::ostream &os, const ComplementSet &cs);

private:
	ComplementSet(FiniteSet *fs) : fs_(fs) {}
	FiniteSet *fs_;
};

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
	os << '{';
	if (fs.size_ > 0) {
		for (FiniteSet::size_type i = 0; i < fs.size_ - 1; ++i) {
			os << fs.elements_[i] << ", ";
		}
		os << fs.elements_[fs.size_ - 1];
	}
	return os << '}';
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
	os << '{';
	if (cs.fs_->size() < cs.fs_->capacity()) {
		auto it = cs.begin();
		os << *it++;

		auto end = cs.end();
		while (it != end) {
			os << ", " << *it++;
		}
	}
	return os << '}';
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
