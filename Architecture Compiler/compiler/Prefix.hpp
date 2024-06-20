#pragma once
#include "Redirection.hpp"
#include "ByteEntry.hpp"
#include <stdint.h>

class Prefix : public ByteEntry
{
public:
	enum Type : uint32_t
	{
		Type_Wait = 1 << 0,
		Type_Repe = 1 << 1,
		Type_Repne = 1 << 2,
		Type_x66 = 1 << 3,
		Type_RexW = 1 << 4,

		Type_Rex = 1 << 5,
		Type_RexB = 1 << 6,
		Type_RexX = 1 << 7,
		Type_RexR = 1 << 8,
		Type_Lock = 1 << 9,
		Type_x67 = 1 << 10,

		Type_CS = 1 << 11,
		Type_SS = 1 << 12,
		Type_DS = 1 << 13,
		Type_ES = 1 << 14,
		Type_GS = 1 << 15,
		Type_FS = 1 << 16,
	};

#pragma pack(push, 1)
	struct Package
	{
		uint16_t m_Instruction; // if pfx is fully redirect we can use it as an instruction for stand alone, 0 if the pfx cannot be standalone
		union
		{
			struct
			{
				uint32_t m_WAIT : 1;
				uint32_t m_REPZ : 1;
				uint32_t m_REPNZ : 1;
				uint32_t m_x66 : 1;
				uint32_t m_REXW : 1;

				uint32_t m_REX : 1;
				uint32_t m_REXB : 1;
				uint32_t m_REXX : 1;
				uint32_t m_REXR : 1;
				uint32_t m_LOCK : 1;
				uint32_t m_x67 : 1;

				uint32_t m_CS : 1;
				uint32_t m_SS : 1;
				uint32_t m_DS : 1;
				uint32_t m_ES : 1;
				uint32_t m_GS : 1;
				uint32_t m_FS : 1;
			};
			uint32_t m_Prefix;
		};
	};
#pragma pack(pop)

public:
	Prefix() = default;
	Prefix(uint32_t prefix);

public:
	bool IsStandAlone();
	void SetStandAlone(uint16_t instructionType);

public:
	PackageType GetPackageType() const override;
	class BytePackage GetPackage(uint32_t freeSpace) const override;

public:
	Redirection::Prefix GetRedirectPrefix() const;

private:
	Package GetBasePackage() const;

private:
	uint32_t m_Prefix = 0;
	uint16_t m_Instruction = 0;
	Redirection::Prefix m_RedirectPrefix = Redirection::Prefix::Default;
};

