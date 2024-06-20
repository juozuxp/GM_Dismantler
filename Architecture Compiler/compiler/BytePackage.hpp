#pragma once
#include "FullRedirection.hpp"
#include "Redirection.hpp"
#include "Instruction.hpp"
#include "Prefix.hpp"

class BytePackage
{
public:
#ifdef INSTRUCTION_PACKING
#pragma pack(push, INSTRUCTION_PACKING)
#else
#pragma pack(push, 1)
#endif
	struct Raw
	{
		Raw() = default;
		Raw(const Prefix::Package& package);
		Raw(const Instruction::Package& package);
		Raw(const Redirection::Package& package);
		Raw(const FullRedirection::Package& package);

		ByteEntry::PackageType m_Type = ByteEntry::PackageType::Invalid;
		union
		{
			Prefix::Package m_Prefix;
			Instruction::Package m_Instruction;
			Redirection::Package m_Redirection;
			FullRedirection::Package m_FullRedirection;
		};
	};
#pragma pack(pop)

public:
	BytePackage() = default;
	BytePackage(const Prefix::Package& package);
	BytePackage(const Instruction::Package& package);
	BytePackage(const Redirection::Package& package);
	BytePackage(const FullRedirection::Package& package);

public:
	uint32_t GetCount() const;
	std::vector<Raw> GetRaw() const;

public:
	void AddChild(const BytePackage& package);

private:
	std::vector<Raw> GetRawChain() const;

private:
	Raw m_Main;
	std::vector<BytePackage> m_Children;
};