#pragma once
#include "x86_x64.hpp"

#pragma pack(push, 1)
enum ILOperandType : uint8_t
{
	ILOperandType_None,
	ILOperandType_Memory,
	ILOperandType_Register,
	ILOperandType_Value,
	ILOperandType_ValueRelative,
	ILOperandType_MemoryRelative,
	ILOperandType_MemoryAbsolute
};
#pragma pack(pop)

#pragma pack(push, 1)
enum ILMemoryScaler : uint8_t
{
	ILMemoryScaler_1,
	ILMemoryScaler_2,
	ILMemoryScaler_4,
	ILMemoryScaler_8
};
#pragma pack(pop)

#pragma pack(push, 1)
enum ILOperandScale : uint8_t
{
	ILOperandScale_Undefined,
	ILOperandScale_8,
	ILOperandScale_16,
	ILOperandScale_32,
	ILOperandScale_64,
	ILOperandScale_80,
	ILOperandScale_128,
	ILOperandScale_256,
	ILOperandScale_512
};
#pragma pack(pop)

#define IL_INVALID_REGISTER static_cast<uint8_t>((1 << 5) - 1)

#pragma pack(push, 1)
struct ILOperand 
{
	struct
	{
		ILOperandType m_Type : 4;
		ILOperandScale m_Scale : 4;
	};
	union
	{
		struct
		{
			uint8_t m_Segment; // segment index, IL_INVALID_REGISTER - reserved for none
			ILMemoryScaler m_Scale;

			uint8_t m_Base; // register index, IL_INVALID_REGISTER - reserved for none
			uint8_t m_Index; // register index, IL_INVALID_REGISTER - reserved for none

			int32_t m_Offset;
		} m_Memory; // ILOperandType_Memory
		struct
		{
			struct
			{
				uint8_t m_Base : 7; // register index, IL_INVALID_REGISTER - reserved for none
				uint8_t m_BaseHigh : 1; // is register high
			};

			Register m_Type;
		} m_Register; // ILOperandType_Register
		struct
		{
			uint8_t m_Segment; // segment index, IL_INVALID_REGISTER - reserved for none
			int64_t m_Value;
		} m_MemoryValue; // [m_Value] if ILOperandType_MemoryAbsolute specified
		struct
		{
			uint8_t m_Segment; // segment index, IL_INVALID_REGISTER - reserved for none
			int32_t m_Value;
		} m_Relative; // rip + m_Value if ILOperandType_ValueRelative, [rip + m_Value] if ILOperandType_MemoryRelative specified

		int64_t m_Value; // ILOperandType_Value
	};
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ILInstruction
{
	InsType m_Type = InsType_invalid;
	uint8_t m_Size;
	ILOperand m_Operands[4];
};
#pragma pack(pop)

#define REG_RAX 0x00 // rax REX extention
#define REG_RCX 0x01 // rcx REX extention
#define REG_RDX 0x02 // rdx REX extention
#define REG_RBX 0x03 // rbx REX extention
#define REG_RSP 0x04 // rsp REX extention
#define REG_RBP 0x05 // rbp REX extention
#define REG_RSI 0x06 // rsi REX extention
#define REG_RDI 0x07 // rdi REX extention

#define REG_EAX 0x00 // eax
#define REG_ECX 0x01 // ecx
#define REG_EDX 0x02 // edx
#define REG_EBX 0x03 // ebx
#define REG_ESP 0x04 // esp
#define REG_EBP 0x05 // ebp
#define REG_ESI 0x06 // esi
#define REG_EDI 0x07 // edi

#define REG_AX 0x00 // ax WORD extention
#define REG_CX 0x01 // cx WORD extention
#define REG_DX 0x02 // dx WORD extention
#define REG_BX 0x03 // bx WORD extention
#define REG_SP 0x04 // sp WORD extention
#define REG_BP 0x05 // bp WORD extention
#define REG_SI 0x06 // si WORD extention
#define REG_DI 0x07 // di WORD extention

#define REG_AL 0x00 // al
#define REG_CL 0x01 // cl
#define REG_DL 0x02 // dl
#define REG_BL 0x03 // bl
#define REG_AH 0x00 // ah
#define REG_CH 0x01 // ch
#define REG_DH 0x02 // dh
#define REG_BH 0x03 // bh

#define REG_SPL 0x04 // spl REX extention
#define REG_BPL 0x05 // bpl REX extention
#define REG_SIL 0x06 // sil REX extention
#define REG_DIL 0x07 // dil REX extention

#define REG_R8 0x08 // r8 REX extention
#define REG_R9 0x09 // r9 REX extention
#define REG_R10 0x0A // r10 REX extention
#define REG_R11 0x0B // r11 REX extention
#define REG_R12 0x0C // r12 REX extention
#define REG_R13 0x0D // r13 REX extention
#define REG_R14 0x0E // r14 REX extention
#define REG_R15 0x0F // r15 REX extention

#define REG_R8D 0x08 // r8d REX extention
#define REG_R9D 0x09 // r9d REX extention
#define REG_R10D 0x0A // r10d REX extention
#define REG_R11D 0x0B // r11d REX extention
#define REG_R12D 0x0C // r12d REX extention
#define REG_R13D 0x0D // r13d REX extention
#define REG_R14D 0x0E // r14d REX extention
#define REG_R15D 0x0F // r15d REX extention

#define REG_R8W 0x08 // r8w REX extention
#define REG_R9W 0x09 // r9w REX extention
#define REG_R10W 0x0A // r10w REX extention
#define REG_R11W 0x0B // r11w REX extention
#define REG_R12W 0x0C // r12w REX extention
#define REG_R13W 0x0D // r13w REX extention
#define REG_R14W 0x0E // r14w REX extention
#define REG_R15W 0x0F // r15w REX extention

#define REG_R8B 0x08 // r8b REX extention
#define REG_R9B 0x09 // r9b REX extention
#define REG_R10B 0x0A // r10b REX extention
#define REG_R11B 0x0B // r11b REX extention
#define REG_R12B 0x0C // r12b REX extention
#define REG_R13B 0x0D // r13b REX extention
#define REG_R14B 0x0E // r14b REX extention
#define REG_R15B 0x0F // r15b REX extention

#define REG_ST0 0x00 // st(0)
#define REG_ST1 0x01 // st(1)
#define REG_ST2 0x02 // st(2)
#define REG_ST3 0x03 // st(3)
#define REG_ST4 0x04 // st(4)
#define REG_ST5 0x05 // st(5)
#define REG_ST6 0x06 // st(6)
#define REG_ST7 0x07 // st(7)

#define REG_MM0 0x00 // mm0
#define REG_MM1 0x01 // mm1
#define REG_MM2 0x02 // mm2
#define REG_MM3 0x03 // mm3
#define REG_MM4 0x04 // mm4
#define REG_MM5 0x05 // mm5
#define REG_MM6 0x06 // mm6
#define REG_MM7 0x07 // mm7

#define REG_XMM0 0x00 // xmm0
#define REG_XMM1 0x01 // xmm1
#define REG_XMM2 0x02 // xmm2
#define REG_XMM3 0x03 // xmm3
#define REG_XMM4 0x04 // xmm4
#define REG_XMM5 0x05 // xmm5
#define REG_XMM6 0x06 // xmm6
#define REG_XMM7 0x07 // xmm7

#define REG_XMM8 0x08 // xmm8 REX extention
#define REG_XMM9 0x09 // xmm9 REX extention
#define REG_XMM10 0x0A // xmm10 REX extention
#define REG_XMM11 0x0B // xmm11 REX extention
#define REG_XMM12 0x0C // xmm12 REX extention
#define REG_XMM13 0x0D // xmm13 REX extention
#define REG_XMM14 0x0E // xmm14 REX extention
#define REG_XMM15 0x0F // xmm15 REX extention

#define REG_CR0 0x00 // cr0
#define REG_CR1 0x01 // cr1
#define REG_CR2 0x02 // cr2
#define REG_CR3 0x03 // cr3
#define REG_CR4 0x04 // cr4
#define REG_CR5 0x05 // cr5
#define REG_CR6 0x06 // cr6
#define REG_CR7 0x07 // cr7

#define REG_CR8 0x08 // cr8 REX extention
#define REG_CR9 0x09 // cr9 REX extention
#define REG_CR10 0x0A // cr10 REX extention
#define REG_CR11 0x0B // cr11 REX extention
#define REG_CR12 0x0C // cr12 REX extention
#define REG_CR13 0x0D // cr13 REX extention
#define REG_CR14 0x0E // cr14 REX extention
#define REG_CR15 0x0F // cr15 REX extention

#define REG_DR0 0x00 // dr0
#define REG_DR1 0x01 // dr1
#define REG_DR2 0x02 // dr2
#define REG_DR3 0x03 // dr3
#define REG_DR4 0x04 // dr4
#define REG_DR5 0x05 // dr5
#define REG_DR6 0x06 // dr6
#define REG_DR7 0x07 // dr7

#define REG_DR8 0x08 // dr8 REX extention
#define REG_DR9 0x09 // dr9 REX extention
#define REG_DR10 0x0A // dr10 REX extention
#define REG_DR11 0x0B // dr11 REX extention
#define REG_DR12 0x0C // dr12 REX extention
#define REG_DR13 0x0D // dr13 REX extention
#define REG_DR14 0x0E // dr14 REX extention
#define REG_DR15 0x0F // dr15 REX extention

#define REG_BND0 0x00 // bnd0
#define REG_BND1 0x01 // bnd1
#define REG_BND2 0x02 // bnd2
#define REG_BND3 0x03 // bnd3

#define REG_CS 0 // cs
#define REG_SS 1 // ss
#define REG_DS 2 // ds
#define REG_ES 3 // es
#define REG_GS 4 // gs
#define REG_FS 5 // fs