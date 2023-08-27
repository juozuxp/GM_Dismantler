#include "AsmRedirectType.hpp"
#include "AsmInstruction.hpp"

AsmRedirectType::AsmRedirectType(uint8_t operand) :
	AsmRedirect(operand)
{
}

void AsmRedirectType::Set(IndexOpType type, std::shared_ptr<AsmIndex> entry)
{
	if (type == IndexOpType::modrm)
	{
		return;
	}

	m_Entries[static_cast<uint8_t>(c_IndexTypeToType[static_cast<uint8_t>(type)])] = entry;
}

std::shared_ptr<const AsmInstruction> AsmRedirectType::GetIndex(const ILInstruction& instruction) const
{
	const ILOperand& operand = instruction.m_Operands[m_Operand];

	uint8_t index = static_cast<uint8_t>(c_ILTypeToType[operand.m_Type]);
	if (operand.m_Type == ILOperandType_MemoryAbsolute &&
		static_cast<uint64_t>(operand.m_MemoryValue.m_Value) < static_cast<uint64_t>(UINT32_MAX))
	{
		index = static_cast<uint8_t>(IndexOpType::mem);
	}

	if (!m_Entries[index])
	{
		return nullptr;
	}

	return m_Entries[index]->GetIndex(instruction);
}