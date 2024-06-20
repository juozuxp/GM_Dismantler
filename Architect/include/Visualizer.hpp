#pragma once
#include "ArchitectureIL.hpp"
#include <string>
#include <vector>

#define VIS_INVALID_REGISTER static_cast<uint8_t>(~0)

class Visualizer
{
	
		//static constexpr std::string_view registers[] = { "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", // General 64
		//											"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d", // General 32
		//											"ax", "cx", "dx", "bx", "sp", "bp", "si", "di", "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w", // General 16
		//											"al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil", "r8l", "r9l", "r10l", "r11l", "r12l", "r13l", "r14l", "r15l", // General 8 lower
		//											"ah", "ch", "dh", "bh", // General 8 upper
		//											"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", // SSE 128 bit
		//											"mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7", "mm8", "mm9", "mm10", "mm11", "mm12", "mm13", "mm14", "mm15", // MMX 64 bit
		//											"bnd0", "bnd1", "bnd2", "bnd3", // MPX 64 bit
		//											"st(0)", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", // FPU 80 bit
		//											"cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7", "cr8", "cr9", "cr10", "cr11", "cr12", "cr13", "cr14", "cr15", // Control registers
		//											"dr0", "dr1", "dr2", "dr3", "dr4", "dr5", "dr6", "dr7", "dr8", "dr9", "dr10", "dr11", "dr12", "dr13", "dr14", "dr15", // Debug registers
		//											"es", "cs", "ss", "fs", "ds", "gs" // Segments
		//											};
	

public:
	struct Options
	{
	};

public:
	enum class VisType : uint8_t
	{
		Instruction,
		OperandMemory,
		OperandMemoryValue,
		OperandAddressValue,
		OperandValue,
		OperandRegister
	};

#pragma pack(push, 1)
	struct Visual
	{
		VisType m_Type;
		union
		{
			uint8_t m_Register;

			struct
			{
				InsType m_Type;
				uint8_t m_Size;
			} m_Instruction;

			struct
			{
				ILOperandScale m_Size;

				uint8_t m_Base;
				uint8_t m_Index;
				uint8_t m_Segment;
				uint8_t m_Multiplier;

				int32_t m_Offset;
			} m_Memory;

			struct
			{
				ILOperandScale m_Size;

				uint8_t m_Segment; // reserved for OperandMemoryValue

				int64_t m_Value;
			} m_Value; // applies to OperandMemoryValue, OperandAddressValue and OperandValue
		};
	};
#pragma pack(pop)

private:
	enum VisRegisterBase
	{
		VisReg_General_64 = 0,
		VisReg_General_32 = 16,
		VisReg_General_16 = 32,
		VisReg_General_8l = 48,
		VisReg_General_8h = 64,
		VisReg_SSE_128 = 68,
		VisReg_MMX_64 = 84,
		VisReg_MPX_64 = 100,
		VisReg_FPU_80 = 104,
		VisReg_CR_64 = 112,
		VisReg_DR_64 = 128,
		VisReg_Sreg_64 = 144,
	};

public:
	Visualizer() = default;
	Visualizer(const Options& options);

public:
	void PrintToConsole(const std::vector<ILInstruction>& instructions, const void* reference) const;

	std::vector<Visual> ToVisuals(const std::vector<ILInstruction>& instructions, const void* reference) const;
	std::vector<std::wstring> ToStrings(const std::vector<ILInstruction>& instructions, const void* reference) const;

private:
	Options m_Options;
};