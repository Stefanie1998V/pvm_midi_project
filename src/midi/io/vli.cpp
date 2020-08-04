#include "vli.h"
#include "io/read.h"
#include <iostream>
namespace io {
	uint64_t read_variable_length_integer(std::istream& in)
	{
		std::vector<uint64_t> numbers;


		char byte;

		io::read_to(in, &byte);
		
		
		uint8_t x = (uint8_t)byte;
		uint8_t old = x;
		while (old >= 128) {
			x = x << 1;
			x = x >> 1;
			numbers.push_back(x);
			//std::cout << std::hex << (int)x << std::endl;
			io::read_to(in, &byte);
			x = ((uint8_t)byte);
			old = x;
			x = x << 1;
			x = x >> 1;
		}
		x = x << 1;
		x = x >> 1;
		//std::cout << std::hex << (int)x << std::endl;
		numbers.push_back(x);


		if (numbers.size() == 1) {
			return numbers[0];
		}

		uint64_t getal = 0;
		for (int i = 0; i < numbers.size(); i++) {
			uint64_t z = numbers[i];
			z = z << (7 * (numbers.size() - 1 - i));
			getal = getal | z;
		}

		//std::cout << getal << std::endl;
		return getal;

	}
}

