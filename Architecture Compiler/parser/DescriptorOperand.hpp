#pragma once
#include <string>
#include <vector>

class DescriptorOperand
{
public:
	enum class Type : uint8_t
	{
		none,
		reg,
		mem,
		modrm,
		imm,
		rel,
		moffs
	};

	enum class Register : uint8_t
	{
		general,
		xmm,
		mm,
		bnd,
		st,
		sreg,
		cr,
		dr
	};

	enum class Size : uint8_t
	{
		undefined,
		base_8,
		far_32,
		base_32,
		base_80,
		base_128,
	};

#pragma pack(push, 1)
	union TypeMask
	{
		struct
		{
			Type m_Type : 3;
			Register m_Register : 5;
		};

		uint8_t m_Value = 0;
	};

	union FlagMask
	{
		struct
		{
			uint8_t m_Rex : 1;
			uint8_t m_Constant : 1;
			uint8_t m_RegisterIndex : 4;
		};

		uint8_t m_Value = 0;
	};

	union SizeMask
	{
		struct
		{
			struct
			{
				Size m_Size : 4;

				uint8_t m_Size16 : 1;
				uint8_t m_Size64 : 1;
				uint8_t m_Size256 : 1;
				uint8_t m_Size512 : 1;
			} m_Reg;

			struct
			{
				Size m_Size : 4;

				uint8_t m_Size16 : 1;
				uint8_t m_Size64 : 1;
				uint8_t m_Size256 : 1;
				uint8_t m_Size512 : 1;
			} m_Mem;
		};

		uint16_t m_Value = 0;
	};
#pragma pack(pop)

public:
	DescriptorOperand() = default;
	DescriptorOperand(const std::vector<std::string>& variations);

public:
	const TypeMask& GetTypeMask() const;
	const SizeMask& GetSizeMask() const;
	const FlagMask& GetFlagMask() const;

private:
	bool ParseForMm(const std::string_view& variation);
	bool ParseForCr(const std::string_view& variation);
	bool ParseForDr(const std::string_view& variation);
	bool ParseForSt(const std::string_view& variation);
	bool ParseForXmm(const std::string_view& variation);
	bool ParseForBnd(const std::string_view& variation);
	bool ParseForSreg(const std::string_view& variation);
	void ParseForGeneral(const std::string_view& variation);

	uint8_t ParseSize(const std::string_view& variation);

private:
	TypeMask m_Type = {};
	SizeMask m_Size = {};
	FlagMask m_Flags = {};

	bool m_IsSeparateSize = false;
};