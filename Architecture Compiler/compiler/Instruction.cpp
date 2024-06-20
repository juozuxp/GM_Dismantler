#include "Instruction.hpp"
#include "BytePackage.hpp"
#include "tools/ArraySize.h"

Instruction::Instruction(const Descriptor& descriptor, uint16_t type) :
	m_Type(type)
{
	const std::vector<DescriptorOperand>& operands = descriptor.GetOperands();

	bool regOnly = false;
	for (const DescriptorOperand& operand : descriptor.GetOperands())
	{
		if (operand.GetTypeMask().m_Type == DescriptorOperand::Type::mem)
		{
			regOnly = false;
			break;
		}
		else if (operand.GetTypeMask().m_Type == DescriptorOperand::Type::reg && !operand.GetFlagMask().m_Constant)
		{
			regOnly = true;
		}
		else if (operand.GetTypeMask().m_Type == DescriptorOperand::Type::modrm)
		{
			regOnly = false;
			break;
		}
	}

	uint8_t count = ARRAY_SIZE(m_Operands) < operands.size() ? ARRAY_SIZE(m_Operands) : operands.size();
	for (uint8_t i = 0; i < count; i++)
	{
		m_Operands[i] = Operand(operands[i], regOnly);
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

std::vector<uint8_t> Instruction::GetCompatiblePrefixes()
{
	std::vector<uint8_t> prefixes;

	bool use66 = false;
	bool useRexW = false;

	for (const Operand& operand : m_Operands)
	{
		if (operand.m_Package.m_Type == Operand::Type::none)
		{
			break;
		}

		if (operand.m_Package.m_Mem.m_Size < DescriptorOperand::Size::base_80)
		{
			use66 = use66 || operand.m_Package.m_Mem.m_Override1;
			useRexW = useRexW || operand.m_Package.m_Mem.m_Override1;
		}

		if (operand.m_Package.m_Reg.m_Size < DescriptorOperand::Size::base_80)
		{
			use66 = use66 || operand.m_Package.m_Reg.m_Override1;
			useRexW = useRexW || operand.m_Package.m_Reg.m_Override1;
		}
	}

	if (use66)
	{
		prefixes.push_back(static_cast<uint8_t>(Redirection::Prefix::x66));
	}

	if (useRexW)
	{
		prefixes.push_back(static_cast<uint8_t>(Redirection::Prefix::RexW));
	}

	return prefixes;
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