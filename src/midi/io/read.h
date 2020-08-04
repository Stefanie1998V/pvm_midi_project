#pragma once
#include <iostream>
#include "logging.h"
#include <memory>

namespace io {

	template<typename T> void read_to(std::istream& in, T* buffer, size_t n = 1) 
	{

		in.read(((char*)buffer), sizeof(T) * n);
		

	}
	template<typename T> std::unique_ptr<T[]> read_array(std::istream& in, size_t n = 1)
	{

		std::unique_ptr<T[]> x = std::make_unique<T[]>(n);

		T buffer;
		for (int i = 0; i < n; i++) {
			read_to(in, &buffer);
			x[i] = buffer;
		}

		return x;


	}


	template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
	T read(std::istream& in)
	{

		T x;
		in.read((char*) &x, sizeof(T));
		return  x;
	}
}

