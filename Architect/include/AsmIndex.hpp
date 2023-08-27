#pragma once
#include <memory>
#include <vector>
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

	enum class IndexOpSize
	{
		reg,
		mem,
		ARRAY_MAX
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
			uint8_t m_Value : 4 = 0;
		};

		OperandSize m_Size[static_cast<uint8_t>(IndexOpSize::ARRAY_MAX)] = {};
	};

	struct Index
	{
		Index() = default;
		Index(const Package& package);

		bool operator==(const Index& lhs);
		bool operator!=(const Index& lhs);


		InsType m_Type = InsType_invalid;

		union
		{
			struct
			{
				uint8_t m_Prefix;
				uint8_t m_Base;
				uint8_t m_X0F383A;
				uint8_t m_Instruction;
				uint8_t m_ModRM;

				uint8_t m_HasPrefix : 1;
				uint8_t m_HasBase : 1;
				uint8_t m_HasX0F383A : 1;
				uint8_t m_HasModRM : 1;
				uint8_t m_OnlyReg : 1;
			};

			uint64_t m_Comparer = 0;
		};

		IndexOperand m_Operands[4] = {};
	};

public:
	static std::shared_ptr<AsmIndex> MapTree(const std::vector<Index>& indexes);

private:
	static std::vector<Index> DeSynonymise(const std::vector<Index>& indexes);

	static std::shared_ptr<AsmIndex> MapOperandType(const std::vector<Index>& indexes, uint8_t first);
	static std::shared_ptr<AsmIndex> MapOperandSize(const std::vector<Index>& indexes, uint8_t first);
	static std::shared_ptr<AsmIndex> MapOperandConstant(const std::vector<Index>& indexes, uint8_t first);
	static std::shared_ptr<AsmIndex> MapOperandRegister(const std::vector<Index>& indexes, uint8_t first);

public:
	virtual std::shared_ptr<const class AsmInstruction> GetIndex(const ILInstruction& instruction) const = 0;
};