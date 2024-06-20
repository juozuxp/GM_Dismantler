#include "Token.hpp"
#include <stdint.h>
#include <algorithm>

Token::Token(const std::string_view& line)
{
	if (line.empty())
	{
		return;
	}

	std::string lineCopy = std::string(line);

	std::transform(lineCopy.begin(), lineCopy.end(), lineCopy.begin(), tolower);

	uint32_t byte = ~0;
	uint32_t seperator = 0;
	for (uint32_t i = 0; i < (lineCopy.size() - 1); i++)
	{
		if (*reinterpret_cast<const uint16_t*>(lineCopy.data() + i) == '>-')
		{
			if (byte != ~0)
			{
				m_Bytes.push_back(std::string(lineCopy.data() + byte, i - byte));
			}

			seperator = i;
			break;
		}

		if (lineCopy[i] == ' ' || lineCopy[i] == '\t')
		{
			if (byte != ~0)
			{
				m_Bytes.push_back(std::string(lineCopy.data() + byte, i - byte));
				byte = ~0;
			}
		}
		else
		{
			if (byte == ~0)
			{
				byte = i;
			}
		}
	}

	if (seperator == 0)
	{
		m_Bytes.clear();
		return;
	}

	for (uint32_t i = seperator + 2;; i++)
	{
		if (i >= lineCopy.size())
		{
			m_Bytes.clear();
			return;
		}

		if (lineCopy[i] != ' ' && lineCopy[i] != '\t')
		{
			seperator = i;
			break;
		}
	}

	uint32_t instruction = seperator;
	for (uint32_t i = seperator; i < lineCopy.size(); i++)
	{
		if (lineCopy[i] == ' ' || lineCopy[i] == '\t')
		{
			seperator = i;
			break;
		}
	}

	if (seperator == instruction)
	{
		seperator = lineCopy.size();
	}

	m_Instruction.assign(lineCopy.data() + instruction, seperator - instruction);
	
	do
	{
		for (uint32_t i = seperator + 1;; i++)
		{
			if (i >= lineCopy.size())
			{
				return;
			}

			if (lineCopy[i] != ' ' && lineCopy[i] != '\t')
			{
				seperator = i;
				break;
			}
		}

		uint32_t operands = seperator;
		for (uint32_t i = seperator; i < lineCopy.size(); i++)
		{
			if (lineCopy[i] == ',')
			{
				seperator = i;
				break;
			}
		}

		if (seperator == operands)
		{
			seperator = lineCopy.size();
		}

		std::vector<std::string> operandTokens;

		uint32_t operand = ~0;
		for (uint32_t i = operands; i < seperator; i++)
		{
			if (lineCopy[i] == ' ' || lineCopy[i] == '\t')
			{
				if (operand != ~0)
				{
					operandTokens.push_back(std::string(lineCopy.data() + operand, i - operand));
					operand = ~0;
				}

				continue;
			}

			if (lineCopy[i] == '|')
			{
				if (operand != ~0)
				{
					operandTokens.push_back(std::string(lineCopy.data() + operand, i - operand));
					operand = ~0;
				}

				continue;
			}

			if (operand == ~0)
			{
				operand = i;
			}
		}

		if (operand != ~0)
		{
			operandTokens.push_back(std::string(lineCopy.data() + operand, seperator - operand));
		}

		m_Operands.push_back(operandTokens);
	} 
	while (seperator != lineCopy.size());
}