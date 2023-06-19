#pragma once
#include "Operand.hpp"
#include "ByteEntry.hpp"
#include "parser/Descriptor.hpp"

class Instruction : public ByteEntry
{
public:
#pragma pack(push, 1)
	struct Package
	{
		uint16_t m_Type;
		Operand::Package m_Operands[4];
	};
#pragma pack(pop)

public:
	Instruction() = default;
	Instruction(const Descriptor& descriptor, uint16_t type);

public:
	PackageType GetPackageType() const override;
	class BytePackage GetPackage(uint32_t freeSpace) const override;

private:
	Package GetBasePackage() const;

private:
	uint16_t m_Type;
	Operand m_Operands[4];
};