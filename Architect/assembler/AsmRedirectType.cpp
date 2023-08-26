#include "AsmRedirectType.hpp"

AsmRedirectType::AsmRedirectType(uint8_t operand) :
	AsmRedirect(operand)
{
}

std::vector<uint8_t> AsmRedirectType::Assemble(const ILInstruction& instruction) const
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
		return std::vector<uint8_t>();
	}

	return m_Entries[index]->Assemble(instruction);
}

void AsmRedirectType::Set(IndexOpType type, std::shared_ptr<AsmIndex> entry)
{
	if (type == IndexOpType::modrm)
	{
		return;
	}

	m_Entries[static_cast<uint8_t>(c_IndexTypeToType[static_cast<uint8_t>(type)])] = entry;
}