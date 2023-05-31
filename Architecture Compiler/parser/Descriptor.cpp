#include "Descriptor.hpp"

Descriptor::Descriptor(const Token& token)
{
	m_Name = token.GetName();

	ParseBytes(token.GetBytes());
	ParseOperands(token.GetOperands());
}

void Descriptor::ParseBytes(const std::vector<std::string>& bytes)
{
	for (std::string byte : bytes)
	{
		if (byte[0] == '/')
		{
			m_UpperOpperand = HexToByte(byte.c_str() + 1);
		}
		else
		{
			m_Bytes.push_back(HexToByte(byte.c_str()));
		}
	}
}

void Descriptor::ParseOperands(const std::vector<std::vector<std::string>>& operands)
{
	for (const std::vector<std::string>& operand : operands)
	{
		m_Operands.push_back(DescriptorOperand(operand));
	}
}