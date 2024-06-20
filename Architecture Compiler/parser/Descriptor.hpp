#pragma once
#include "lexer/Token.hpp"
#include "DescriptorOperand.hpp"

class Descriptor
{
public:
	Descriptor() = default;

	Descriptor(const Token& token);

public:
	uint8_t GetUpperOperand() const;

	const std::string& GetName() const;
	const std::vector<uint8_t>& GetBytes() const;
	const std::vector<DescriptorOperand>& GetOperands() const;

private:
	static constexpr uint8_t HexToByte(const char* hex)
	{
		uint8_t upper = HexToNum(hex[0]);
		if ((upper & (1 << 4)) != 0)
		{
			return 0;
		}

		uint8_t lower = HexToNum(hex[1]);
		if ((lower & (1 << 4)) != 0)
		{
			return upper;
		}

		return (upper << 4) | lower;
	}

	static constexpr uint8_t HexToNum(char hex)
	{
		if (hex >= '0' && hex <= '9')
		{
			return hex - '0';
		}

		if (hex >= 'a' && hex <= 'f')
		{
			return hex - 'a' + 10;
		}

		if (hex >= 'A' && hex <= 'F')
		{
			return hex - 'A' + 10;
		}

		return 1 << 4;
	}

private:
	void ParseBytes(const std::vector<std::string>& bytes);
	void ParseOperands(const std::vector<std::vector<std::string>>& operands);

private:
	std::string m_Name;

	uint8_t m_UpperOpperand = ~0;

	std::vector<uint8_t> m_Bytes;
	std::vector<DescriptorOperand> m_Operands;
};

