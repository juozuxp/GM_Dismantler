#pragma once
#include "Instruction.hpp"
#include "Redirection.hpp"
#include "Prefix.hpp"

class ByteEntry
{
public:
	enum class PackageType
	{
		Invalid,
		Prefix,
		Instruction,
		Redirection
	};

#ifdef INSTRUCTION_PACKING
#pragma pack(push, INSTRUCTION_PACKING)
#else
#pragma pack(push, 1)
#endif
	struct Package
	{
		PackageType m_Type;
		union
		{
			Prefix::Package m_Prefix;
			Prefix::Package m_Instruction;
			Prefix::Package m_Redirection;
		};
	};
#pragma pack(pop)
public:
	ByteEntry() = default;

	ByteEntry(const Prefix& prefix);
	ByteEntry(const Instruction& instruction);
	ByteEntry(const Redirection& redirection);

public:
	Package GetPackage() const;

private:
	PackageType m_Type = PackageType::Invalid;
	union
	{
		Prefix m_Prefix;
		Instruction m_Instruction;
		Redirection m_Redirection;
	};
};