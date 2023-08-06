#pragma once
#include <stdint.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

class Utility
{
public:
	static constexpr uint8_t CountToBit(uint32_t value)
	{
		if (value == 0)
		{
			return ~0;
		}

		for (uint8_t i = 0;; i++, value >>= 1)
		{
			if (value & 1)
			{
				return i;
			}
		}

		return ~0; // impossible value return, here to shut the compiler up
	}

	static constexpr uint8_t BitSize(uint32_t value)
	{
		uint32_t copy = value;
		uint8_t count = 0;

		for (count = 0; copy != 0; count++)
		{
			copy >>= 1;
		}

		return count + ((value & ((1 << (count - 1)) - 1)) != 0 ? 1 : 0);
	}
};