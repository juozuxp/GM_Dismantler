#include "AsmRedirectType.hpp"

AsmRedirectType::AsmRedirectType(uint8_t operand) :
	AsmRedirect(operand)
{
}

std::shared_ptr<const AsmIndex> AsmRedirectType::GetEntry(const ILInstruction& instruction) const
{
	const ILOperand& operand = instruction.m_Operands[m_Operand];

	uint8_t index = static_cast<uint8_t>(c_ILTypeToType[operand.m_Type]);
	if (operand.m_Type == ILOperandType_MemoryAbsolute &&
		operand.m_Scale != ILOperandScale_64)
	{
		index = static_cast<uint8_t>(IndexOpType::mem);
	}

	if (!m_Entries[index])
	{
		return std::shared_ptr<AsmIndex>();
	}

	return m_Entries[index]->GetEntry(instruction);
}

void AsmRedirectType::Set(IndexOpType type, std::shared_ptr<AsmIndex> entry)
{
	if (type == IndexOpType::modrm)
	{
		return;
	}

	m_Entries[static_cast<uint8_t>(c_IndexTypeToType[static_cast<uint8_t>(type)])] = entry;
}