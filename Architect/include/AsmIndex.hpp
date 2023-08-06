#pragma once
#include <memory>
#include "ArchitectureIL.hpp"

class AsmIndex : public std::enable_shared_from_this<AsmIndex>
{
public:
	enum class IndexOpType : uint8_t
	{
		none,
		reg,
		mem,
		modrm,
		imm,
		rel,
		moffs
	};

	struct IndexOperand
	{
		IndexOperand() = default;
		IndexOperand(const Operand& operand);

		struct
		{
			IndexOpType m_Type : 3 = IndexOpType::none;
			Register m_Register : 5 = Register::general;
		};

		struct
		{
			uint8_t m_Rex : 1 = 0;
			uint8_t m_Constant : 1 = 0;
			uint8_t m_RegisterIndex : 4 = 0;
		};

		OperandSize m_Reg = {};
		OperandSize m_Mem = {};
	};

	struct Index
	{
		Index() = default;
		Index(const Package& package);

		InsType m_Type = InsType_invalid;

		uint8_t m_Prefix = 0;
		uint8_t m_Bytes[4] = {};

		uint8_t n_HasUpper : 1 = false;
		uint8_t m_HasPrefix : 1 = false;

		uint8_t m_ByteCount : 3 = 0;
		uint8_t m_UpperOperand : 3 = 0;

		IndexOperand m_Operands[4] = {};
	};

public:
	virtual std::shared_ptr<AsmIndex> GetEntry(const ILInstruction& instruction) = 0;
};