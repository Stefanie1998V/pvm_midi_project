#include "endianness.h"
#include <iostream>

namespace io {

	


	void switch_endianness(uint16_t* n)
	{

		uint16_t x1 = (*n) << 8;
		uint16_t x2 = (*n) >> 8;

		*n = x1 | x2;

	}

	void switch_endianness(uint32_t* n)
	{
		uint32_t x1 = (*n) << 24;
		uint32_t x2 = (*n) >> 8 << 24 >> 8;
		uint32_t x3 = (*n) << 8 >> 24 << 8;
		uint32_t x4 = (*n) >> 24;

		*n = x1 | x2 | x3 | x4;

	}

	void switch_endianness(uint64_t* n)
	{
		uint64_t x8 = (*n) >> 56;
		uint64_t x7 = (*n) << 8 >> 56 << 8;
		uint64_t x6 = (*n) << 16 >> 56 << 16;
		uint64_t x5 = (*n) << 24 >> 56 << 24;
		uint64_t x4 = (*n) << 32 >> 56 << 32;
		uint64_t x3 = (*n) << 40 >> 56 << 40;
		uint64_t x2 = (*n) << 48 >> 56 << 48;
		uint64_t x1 = (*n) << 56;



		*n = x1 | x2 | x3 | x4 | x5 | x6 | x7 | x8;

	}
}
