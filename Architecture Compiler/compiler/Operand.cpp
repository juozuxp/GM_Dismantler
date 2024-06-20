#include "Operand.hpp"
#include <unordered_map>

Operand::Operand(const DescriptorOperand& descriptor, bool regOnly)
{
	static const std::unordered_map<DescriptorOperand::Type, Type> typeMap = []() -> std::unordered_map<DescriptorOperand::Type, Type>
	{
		std::unordered_map<DescriptorOperand::Type, Type> map;

		map[DescriptorOperand::Type::reg] = Type::reg;
		map[DescriptorOperand::Type::mem] = Type::modrm;
		map[DescriptorOperand::Type::modrm] = Type::modrm;
		map[DescriptorOperand::Type::imm] = Type::imm;
		map[DescriptorOperand::Type::rel] = Type::rel;
		map[DescriptorOperand::Type::moffs] = Type::moffs;

		return map;
	}();

	const DescriptorOperand::SizeMask& size = descriptor.GetSizeMask();
	const DescriptorOperand::TypeMask& type = descriptor.GetTypeMask();
	const DescriptorOperand::FlagMask& flags = descriptor.GetFlagMask();

	m_Package.m_Register = type.m_Register;
	m_Package.m_Type = typeMap.at(type.m_Type);

	m_Package.m_Rex = flags.m_Rex;
	m_Package.m_Constant = flags.m_Constant;
	m_Package.m_RegisterIndex = flags.m_RegisterIndex;

	m_Package.m_Mem = size.m_Mem;
	m_Package.m_Reg = size.m_Reg;

	if (regOnly && m_Package.m_Type == Type::reg && !flags.m_Constant)
	{
		m_Package.m_Type = Type::modrm;
	}
}

const Operand::Package& Operand::GetPackage() const
{
	return m_Package;
}