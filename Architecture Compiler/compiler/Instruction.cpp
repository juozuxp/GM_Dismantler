#include "Instruction.hpp"
#include "BytePackage.hpp"
#include "tools/ArraySize.h"

Instruction::Instruction(const Descriptor& descriptor, uint16_t type) :
	m_Type(type)
{
	const std::vector<DescriptorOperand>& operands = descriptor.GetOperands();

	uint8_t count = ARRAY_SIZE(m_Operands) < operands.size() ? ARRAY_SIZE(m_Operands) : operands.size();
	for (uint8_t i = 0; i < count; i++)
	{
		m_Operands[i] = Operand(operands[i]);
	}
}

Instruction::PackageType Instruction::GetPackageType() const
{
	return PackageType::Instruction;
}

BytePackage Instruction::GetPackage(uint32_t freeSpace) const
{
	return Package(GetBasePackage());
}

Instruction::Package Instruction::GetBasePackage() const
{
	Package package = {};

	package.m_Type = m_Type;
	for (uint32_t i = 0; i < ARRAY_SIZE(m_Operands); i++)
	{
		package.m_Operands[i] = m_Operands[i].GetPackage();
	}

	return package;
}