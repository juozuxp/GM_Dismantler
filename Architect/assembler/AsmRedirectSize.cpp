#include "AsmRedirectSize.hpp"
#include "AsmInstruction.hpp"

AsmRedirectSize::AsmRedirectSize(uint8_t operand) :
	AsmRedirect(operand)
{
}

void AsmRedirectSize::Set(OpSize size, std::shared_ptr<AsmIndex> entry)
{
	m_Entries[static_cast<uint8_t>(size)] = entry;
}

std::shared_ptr<const AsmInstruction> AsmRedirectSize::GetIndex(const ILInstruction& instruction) const
{
	const ILOperand& operand = instruction.m_Operands[m_Operand];

	uint8_t index = static_cast<uint8_t>(c_ILToOpSizeMap[operand.m_Scale]);
	if (!m_Entries[index])
	{
		return nullptr;
	}

	return m_Entries[index]->GetIndex(instruction);
}