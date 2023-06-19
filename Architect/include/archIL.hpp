#pragma once
#include "x86_x64.hpp"

#pragma pack(push, 1)
enum ILOperandType : uint8_t
{
	ILOperandType_None,
	ILOperandType_Memory,
	ILOperandType_Register,
	ILOperandType_Value,
	ILOperandType_MemoryRelative,
	ILOperandType_MemoryValue
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
struct ILOperand 
{
	ILOperandType m_Type;
	union
	{
		struct
		{
			ILMemoryScaler m_Scale;

			uint8_t m_First; // register index
			uint8_t m_Second; // register index

			uint32_t m_Offset; // relative offset if ILOperandType_MemoryRelative
		} m_Memory; // ILOperandType_Memory, ILOperandType_MemoryRelative
		struct
		{
			uint8_t m_Index;
		} m_Register; // ILOperandType_Register
		struct
		{
			uint64_t m_Value; // if ILOperandType_MemoryValue specified, [m_Value]
		} m_Value; // ILOperandType_Value, ILOperandType_MemoryValue
	};
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ILInstruction
{
	InstructionType m_Type = Instruction_none;
	ILOperand m_Operands[4];
};
#pragma pack(pop)