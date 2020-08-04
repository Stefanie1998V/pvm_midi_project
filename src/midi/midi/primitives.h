#pragma once
#include "util/tagged.h"

namespace midi {


	class __declspec(empty_bases)Channel : public tagged<uint8_t, Channel>, equality<Channel>, show_value<Channel, int>
	{
		using tagged::tagged; 
	};

	class __declspec(empty_bases)Instrument : public tagged<uint8_t, Instrument>, equality<Instrument>, show_value<Instrument, int>
	{
		using tagged::tagged;
	};

	class __declspec(empty_bases)NoteNumber : public tagged<uint8_t, NoteNumber>, ordered<NoteNumber>, show_value<NoteNumber, int>
	{
		using tagged::tagged;
	};


	class __declspec(empty_bases)Duration : public tagged<uint64_t, Duration>, ordered<Duration>, show_value<Duration, int>
	{
	public:
		using tagged::tagged;

		Duration operator +(const Duration& d1) const{
			return Duration(this->_value + d1._value);
		}

		Duration operator -(const Duration& d1) const {
			return Duration(this->_value - d1._value);
		}

		Duration& operator +=(const Duration& d1) {
			this->_value += d1._value;
			return *this;
		}

		Duration& operator -=(const Duration& d1) {
			this->_value -= d1._value;
			return *this;
		}

	};

	class __declspec(empty_bases)Time : public tagged<uint64_t, Time>, ordered<Time>, show_value<Time, int>
	{
	public:
		using tagged::tagged;

		

		Duration operator -(const Time& t1) const{
			return Duration(this->_value - t1._value);
		}
		Time operator +(const Duration& d1) const {
			return Time(this->_value + d1._value);
		}
		
		Time& operator +=(const Duration& d1) {
			this->_value += d1._value;
			return *this;
		}


		Time& operator -=(const Duration& d1) {
			this->_value -= d1._value;
			return *this;
		}


	};

	inline Time operator +(const Duration& d1, const Time& t1 ){
		return Time(t1._value + d1._value);
	}
}