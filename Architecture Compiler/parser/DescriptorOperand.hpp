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
		far_32, // override1 -> override to size 16, override2 -> override to size 64
		base_8,
		base_16, // override1 -> override to size 32, override2 -> override to size 64
		base_32, // override1 -> override to size 16, override2 -> override to size 64
		base_64, // override1 -> override to size 16
		base_80,
		base_128, // override1 -> override to size 256, override2 -> override to size 512
		base_256,
		base_512
	};

	enum class Segments : uint8_t // Segment denotation, for the m_RegisterIndex
	{
		es,
		cs,
		ss,
		ds,
		fs,
		gs
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

	union OperandSize
	{
		struct
		{
			Size m_Size : 6;

			uint8_t m_Override1 : 1; // x66 or ymm override
			uint8_t m_Override2 : 1; // x48 (rexw) or zmm override
		};

		uint8_t m_Value;
	};

	union SizeMask
	{
		struct
		{
			OperandSize m_Reg;
			OperandSize m_Mem;
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

	void AssignSize(OperandSize* mask, uint32_t size);

private:
	TypeMask m_Type = {};
	SizeMask m_Size = {};
	FlagMask m_Flags = {};

	bool m_IsSeparateSize = false;
};