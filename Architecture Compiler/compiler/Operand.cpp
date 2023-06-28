#include "Operand.hpp"
#include <unordered_map>

Operand::Operand(const DescriptorOperand& descriptor)
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

	m_Package.m_Mem.m_Size = size.m_Mem.m_Size;
	m_Package.m_Mem.m_Size16 = size.m_Mem.m_Size16;
	m_Package.m_Mem.m_Size64 = size.m_Mem.m_Size64;
	m_Package.m_Mem.m_Size256 = size.m_Mem.m_Size256;
	m_Package.m_Mem.m_Size512 = size.m_Mem.m_Size512;

	m_Package.m_Reg.m_Size = size.m_Reg.m_Size;
	m_Package.m_Reg.m_Size16 = size.m_Reg.m_Size16;
	m_Package.m_Reg.m_Size64 = size.m_Reg.m_Size64;
	m_Package.m_Reg.m_Size256 = size.m_Reg.m_Size256;
	m_Package.m_Reg.m_Size512 = size.m_Reg.m_Size512;
}

const Operand::Package& Operand::GetPackage() const
{
	return m_Package;
}