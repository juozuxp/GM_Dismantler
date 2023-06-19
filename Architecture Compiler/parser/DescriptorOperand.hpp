#pragma once
#include <string>
#include <vector>

class DescriptorOperand
{
public:
	enum class Type : uint8_t
	{
		reg,
		mem,
		modrm,
		imm,
		rel
	};

	enum class Register : uint8_t
	{
		general,
		xmm,
		mm,
		bnd,
		st
	};

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
			uint8_t m_Size8 : 1;
			uint8_t m_Size16 : 1;
			uint8_t m_Size32 : 1;
			uint8_t m_Size64 : 1;
			uint8_t m_Size128 : 1;
			uint8_t m_Size256 : 1;
			uint8_t m_Size512 : 1;
		};

		uint8_t m_Value = 0;
	};

public:
	DescriptorOperand() = default;
	DescriptorOperand(const std::vector<std::string>& variations);

public:
	const TypeMask& GetTypeMask() const;
	const SizeMask& GetSizeMask() const;
	const FlagMask& GetFlagMask() const;

private:
	bool ParseForMm(const std::string& variation);
	bool ParseForSt(const std::string& variation);
	bool ParseForXmm(const std::string& variation);
	bool ParseForBnd(const std::string& variation);
	void ParseForGeneral(const std::string& variation);

	void ParseSize(const std::string_view& variation);

private:
	TypeMask m_Type;
	SizeMask m_Size;
	FlagMask m_Flags;
};

