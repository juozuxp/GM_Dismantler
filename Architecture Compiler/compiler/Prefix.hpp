#pragma once
#include "Redirection.hpp"
#include "ByteEntry.hpp"
#include <stdint.h>

class Prefix : public ByteEntry
{
public:
	enum Type : uint16_t
	{
		Type_RexW = 1 << 0,
		Type_Wait = 1 << 1,
		Type_Repe = 1 << 2,
		Type_Repne = 1 << 3,
		Type_x66 = 1 << 4,
		Type_x0F38 = 1 << 5,
		Type_x0F3A = 1 << 6,

		Type_Rex = 1 << 7,
		Type_RexB = 1 << 8,
		Type_RexX = 1 << 9,
		Type_RexR = 1 << 10,
		Type_GS = 1 << 11,
		Type_FS = 1 << 12,
		Type_Lock = 1 << 13
	};

#pragma pack(push, 1)
	union Package
	{
		struct
		{
			uint16_t m_REXW : 1;
			uint16_t m_WAIT : 1;
			uint16_t m_REPZ : 1;
			uint16_t m_REPNZ : 1;
			uint16_t m_x66 : 1;
			uint16_t m_x0F38 : 1;
			uint16_t m_x0F3A : 1;

			uint16_t m_REX : 1;
			uint16_t m_REXB : 1;
			uint16_t m_REXX : 1;
			uint16_t m_REXR : 1;
			uint16_t m_GS : 1;
			uint16_t m_FS : 1;
			uint16_t m_LOCK : 1;
		};
		uint16_t m_Value;
	};
#pragma pack(pop)

public:
	Prefix() = default;
	Prefix(uint16_t prefix);

public:
	PackageType GetPackageType() const override;
	class BytePackage GetPackage(uint32_t freeSpace) const override;

public:
	Redirection::Prefix GetRedirectPrefix() const;

private:
	Package GetBasePackage() const;

private:
	uint16_t m_Prefix = 0;
	Redirection::Prefix m_RedirectPrefix = Redirection::Prefix::Default;
};

