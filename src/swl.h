/*
 * @author = Bc. David Pivovar
 */

#pragma once

#include<deque>

template <class T >
class swl : public std::deque<T>
{
public:
	//swl<T>() = delete;
	swl<T>() : swl<T>(12) {};
	explicit swl<T>(size_t width) : _width(width) {};

	inline void push_back(T val) {
		std::deque<T>::push_back(val);
		if (std::deque<T>::size() > _width) std::deque<T>::pop_front();
	}

	inline void push_front(T val) {
		std::deque<T>::push_front(val);
		if (std::deque<T>::size() > _width) std::deque<T>::pop_back();
	}

	inline std::vector<T> to_vector() {
		return std::vector(std::deque<T>::begin(), std::deque<T>::end());
	}

	const size_t _width;
};