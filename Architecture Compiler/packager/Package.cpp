#include "Package.hpp"

#define NEW_LINE_BREAK 0x10

Package::Package(const InstructionSet& instructions)
{
	std::vector<BytePackage::Raw> raw = instructions.GetPackage().GetRaw();

	m_ByteCode.resize(raw.size() * sizeof(BytePackage::Raw));
	memcpy(m_ByteCode.data(), raw.data(), raw.size() * sizeof(BytePackage::Raw));

	m_Types = instructions.GetTypes();
}

void Package::OutputToHeader(const std::string_view& filePath) const
{
	std::ofstream stream = std::ofstream(filePath.data());

	stream << "#pragma once\n#include <stdint.h>\n\n" + GetCoreStructures() + "\n\n" + GetArrayTypeStrings() + "\n\n" + GetArrayByteCode();
}

std::string Package::GetEnumTypes() const
{
	std::string enumerator = "enum InsType : uint16_t\n{\n";

	for (const std::string& type : m_Types)
	{
		enumerator += "\tInsType_";
		enumerator += type;
		enumerator += ",\n";
	}

	return enumerator + "\tInsType_ARRAY_MAX\n};";
}

std::string Package::GetCoreStructures() const
{
#ifdef INSTRUCTION_PACKING
	std::string CorePacking = "#define INSTRUCTION_PACKING " + std::to_string(INSTRUCTION_PACKING);
#else
	std::string CorePacking = "#define INSTRUCTION_PACKING 1";
#endif

	std::string CoreInstruction =
	{
		"enum class Register : uint8_t\n"
		"{\n"
		"	general,\n"
		"	xmm,\n"
		"	mm,\n"
		"	bnd,\n"
		"	st,\n"
		"	sreg,\n"
		"	cr,\n"
		"	dr\n"
		"};\n"
		"\n"
		"enum class OpSize : uint8_t\n"
		"{\n"
		"	undefined,\n"
		"	far_32, // override1 -> override to size 16, override2 -> override to size 64\n"
		"	base_8,\n"
		"	base_16, // override1 -> override to size 32, override2 -> override to size 64\n"
		"	base_32, // override1 -> override to size 16, override2 -> override to size 64\n"
		"	base_64, // override1 -> override to size 16\n"
		"	base_80,\n"
		"	base_128, // override1 -> override to size 256, override2 -> override to size 512\n"
		"	base_256,\n"
		"	base_512\n"
		"};\n"
		"\n"
		"enum class OpType : uint8_t\n"
		"{\n"
		"	none,\n"
		"	reg,\n"
		"	modrm,\n"
		"	imm,\n"
		"	rel,\n"
		"	moffs\n"
		"};\n"
		"\n"
		"#pragma pack(push, 1)\n"
		"union OperandSize\n"
		"{\n"
		"	struct\n"
		"	{\n"
		"		OpSize m_Size : 6;\n"
		"\n"
		"		uint8_t m_Override1 : 1; // x66 or ymm override\n"
		"		uint8_t m_Override2 : 1; // x48 (rexw) or zmm override\n"
		"	};\n"
		"\n"
		"	uint8_t m_Value;\n"
		"};\n"
		"\n"
		"struct Operand\n"
		"{\n"
		"	struct\n"
		"	{\n"
		"		OpType m_Type : 3;\n"
		"		Register m_Register : 5;\n"
		"	};\n"
		"	\n"
		"	struct\n"
		"	{\n"
		"		uint8_t m_Rex : 1;\n"
		"		uint8_t m_Constant : 1;\n"
		"		uint8_t m_Value : 4;\n"
		"	};\n"
		"	\n"
		"	OperandSize m_Reg;\n"
		"	OperandSize m_Mem;\n"
		"};\n"
		"\n"
		"struct Instruction\n"
		"{\n"
		"	InsType m_Type;\n"
		"	Operand m_Operands[4];\n"
		"};\n"
		"#pragma pack(pop)"
	};

	std::string CorePrefix =
	{
		"enum PfxType : uint32_t\n"
		"{\n"
		"	Prefix_Wait = 1 << 0,\n"
		"	Prefix_Repe = 1 << 1,\n"
		"	Prefix_Repne = 1 << 2,\n"
		"	Prefix_x66 = 1 << 3,\n"
		"	Prefix_RexW = 1 << 4,\n"
		"	\n"
		"	Prefix_Rex = 1 << 5,\n"
		"	Prefix_RexB = 1 << 6,\n"
		"	Prefix_RexX = 1 << 7,\n"
		"	Prefix_RexR = 1 << 8,\n"
		"	Prefix_Lock = 1 << 9,\n"
		"	Prefix_x67 = 1 << 10,\n"
		"	\n"
		"	Prefix_CS = 1 << 11,\n"
		"	Prefix_SS = 1 << 12,\n"
		"	Prefix_DS = 1 << 13,\n"
		"	Prefix_ES = 1 << 14,\n"
		"	Prefix_GS = 1 << 15,\n"
		"	Prefix_FS = 1 << 16,\n"
		"};\n"
		"\n"
		"#pragma pack(push, 1)\n"
		"struct Prefix\n"
		"{\n"
		"	InsType m_Instruction; // if pfx is fully redirect we can use it as an instruction for stand alone, InsType_invalid if the pfx cannot be standalone\n"
		"	union\n"
		"	{\n"
		"		struct\n"
		"		{\n"
		"			uint32_t m_WAIT : 1;\n"
		"			uint32_t m_REPZ : 1;\n"
		"			uint32_t m_REPNZ : 1;\n"
		"			uint32_t m_x66 : 1;\n"
		"			uint32_t m_REXW : 1;\n"
		"			\n"
		"			uint32_t m_REX : 1;\n"
		"			uint32_t m_REXB : 1;\n"
		"			uint32_t m_REXX : 1;\n"
		"			uint32_t m_REXR : 1;\n"
		"			uint32_t m_LOCK : 1;\n"
		"			uint32_t m_x67 : 1;\n"
		"			\n"
		"			uint32_t m_CS : 1;\n"
		"			uint32_t m_SS : 1;\n"
		"			uint32_t m_DS : 1;\n"
		"			uint32_t m_ES : 1;\n"
		"			uint32_t m_GS : 1;\n"
		"			uint32_t m_FS : 1;\n"
		"		};\n"
		"		uint32_t m_Prefix;\n"
		"	};\n"
		"};\n"
		"#pragma pack(pop)"
	};

	std::string CoreRedirect =
	{
		"#define REDIRECT_REG_SIZE 8\n"
		"#define REDIRECT_MEM_SIZE 8\n"
		"#define REDIRECT_MOD_SIZE 4\n"
		"#define REDIRECT_PREFIX_SIZE 6\n"
		"#define REDIRECT_X0F383A_SIZE 3\n"
		"\n"
		"enum class ReType : uint8_t\n"
		"{\n"
		"	None,\n"
		"	X0F383A,\n"
		"	Prefix,\n"
		"	Mod,\n"
		"	Reg,\n"
		"	Mem\n"
		"};\n"
		"\n"
		"enum X0F383A : uint8_t\n"
		"{\n"
		"	X0F383A_None,\n"
		"	X0F383A_0F38,\n"
		"	X0F383A_0F3A\n"
		"};\n"
		"\n"
		"#pragma pack(push, 1)\n"
		"struct Redirect\n"
		"{\n"
		"	ReType m_Type;\n"
		"	uint32_t m_BaseIndex;\n"
		"	union\n"
		"	{\n"
		"		uint32_t m_Mask;\n"
		"		union\n"
		"		{\n"
		"			uint32_t m_Value;\n"
		"			struct\n"
		"			{\n"
		"				uint32_t m_Valid00 : 1;\n"
		"				uint32_t m_Valid01 : 1;\n"
		"				uint32_t m_Valid10 : 1;\n"
		"				uint32_t m_Valid11 : 1;\n"
		"				\n"
		"				uint32_t m_Index00 : 2;\n"
		"				uint32_t m_Index01 : 2;\n"
		"				uint32_t m_Index10 : 2;\n"
		"				uint32_t m_Index11 : 2;\n"
		"			};\n"
		"		} m_Mod;\n"
		"		union\n"
		"		{\n"
		"			uint32_t m_Value;\n"
		"			struct\n"
		"			{\n"
		"				uint32_t m_Valid000 : 1;\n"
		"				uint32_t m_Valid001 : 1;\n"
		"				uint32_t m_Valid010 : 1;\n"
		"				uint32_t m_Valid011 : 1;\n"
		"				uint32_t m_Valid100 : 1;\n"
		"				uint32_t m_Valid101 : 1;\n"
		"				uint32_t m_Valid110 : 1;\n"
		"				uint32_t m_Valid111 : 1;\n"
		"				\n"
		"				uint32_t m_Index000 : 3;\n"
		"				uint32_t m_Index001 : 3;\n"
		"				uint32_t m_Index010 : 3;\n"
		"				uint32_t m_Index011 : 3;\n"
		"				uint32_t m_Index100 : 3;\n"
		"				uint32_t m_Index101 : 3;\n"
		"				uint32_t m_Index110 : 3;\n"
		"				uint32_t m_Index111 : 3;\n"
		"			};\n"
		"		} m_RegRM;\n"
		"		union\n"
		"		{\n"
		"			uint32_t m_Value;\n"
		"			struct\n"
		"			{\n"
		"				uint32_t m_ValidWait : 1;\n"
		"				uint32_t m_ValidRepe : 1;\n"
		"				uint32_t m_ValidRepne : 1;\n"
		"				uint32_t m_Valid66 : 1;\n"
		"				uint32_t m_ValidRexW : 1;\n"
		"				uint32_t m_ValidDefault : 1;\n"
		"				uint32_t m_ValidUnused : 2;\n"
		"				\n"
		"				uint32_t m_IndexRexW : 3;\n"
		"				uint32_t m_IndexWait : 3;\n"
		"				uint32_t m_IndexRepe : 3;\n"
		"				uint32_t m_IndexRepne : 3;\n"
		"				uint32_t m_Index66 : 3;\n"
		"				uint32_t m_IndexDefault : 3;\n"
		"			};\n"
		"		} m_Prefix;\n"
		"		union\n"
		"		{\n"
		"			uint32_t m_Value;\n"
		"			struct\n"
		"			{\n"
		"				uint32_t m_ValidDefault : 1;\n"
		"				uint32_t m_Valid0F38 : 1;\n"
		"				uint32_t m_Valid0F3A : 1;\n"
		"				uint32_t m_ValidUnused : 1;\n"
		"				\n"
		"				uint32_t m_IndexDefault : 2;\n"
		"				uint32_t m_Index0F38 : 2;\n"
		"				uint32_t m_Index0F3A : 2;\n"
		"			};\n"
		"		} m_x0F383A;\n"
		"	};\n"
		"};\n"
		"#pragma pack(pop)"
	};

	std::string CoreFullRedirect =
	{
		"#pragma pack(push, 1)\n"
		"struct FullRedirect\n"
		"{\n"
		"	uint16_t m_BaseIndex;\n"
		"};\n"
		"#pragma pack(pop)"
	};

	std::string CorePackage =
	{
		"enum class PaType : uint8_t\n"
		"{\n"
		"	Invalid,\n"
		"	Prefix,\n"
		"	Redirect,\n"
		"	Instruction,\n"
		"	FullRedirect\n"
		"};\n"
		"\n"
		"#pragma pack(push, INSTRUCTION_PACKING)\n"
		"struct Package\n"
		"{\n"
		"	PaType m_Type;\n"
		"	union\n"
		"	{\n"
		"		Prefix m_Prefix;\n"
		"		Redirect m_Redirect;\n"
		"		Instruction m_Instruction;\n"
		"		FullRedirect m_FullRedirect;\n"
		"	};\n"
		"};\n"
		"#pragma pack(pop)"
	};

	return CorePacking + "\n\n" + GetEnumTypes() + "\n\n" + CoreInstruction + "\n\n" + CorePrefix + "\n\n" + CoreRedirect + "\n\n" + CoreFullRedirect + "\n\n" + CorePackage;
}

std::string Package::GetArrayByteCode() const
{
	std::string array = "constexpr static uint8_t CompiledPackage[] = \n{\n\t";

	uint8_t newLine = 0;
	for (uint8_t byte : m_ByteCode)
	{
		if (NEW_LINE_BREAK <= newLine)
		{
			newLine = 1;

			array.erase(array.size() - 1);
			array += "\n\t";
		}
		else
		{
			newLine++;
		}

		char first = (byte >> 4);
		char second = (byte & ((1 << 4) - 1));

		first += first < 10 ? '0' : ('A' - 10);
		second += second < 10 ? '0' : ('A' - 10);

		array += "0x";
		array += first;
		array += second;

		array += ", ";
	}


	array.erase(array.size() - 2);
	return array + "\n};";
}

std::string Package::GetArrayTypeStrings() const
{
	std::string array = "constexpr static const char* TypeToName[] = \n{\n\t";

	uint8_t newLine = 0;
	for (const std::string& type : m_Types)
	{
		if (NEW_LINE_BREAK <= newLine)
		{
			newLine = 1;

			array.erase(array.size() - 1);
			array += "\n\t";
		}
		else
		{
			newLine++;
		}

		array += "\"";
		array += type;
		array += "\", ";
	}

	array.erase(array.size() - 2);
	return array + "\n};";
}