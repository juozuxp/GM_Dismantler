#include "AsmRedirectConstant.hpp"
#include "Utility.hpp"

AsmRedirectConstant::AsmRedirectConstant(uint8_t operand) :
	AsmRedirect(operand)
{
}

void AsmRedirectConstant::Set(uint8_t value, std::shared_ptr<AsmIndex> index)
{
	if (value == static_cast<uint8_t>(~0))
	{
		m_Entries[0] = index;
	}
	else
	{
		m_Entries[value + 1] = index;
	}
}

std::shared_ptr<const AsmIndex> AsmRedirectConstant::GetEntry(const ILInstruction& instruction) const
{
	const ILOperand& operand = instruction.m_Operands[m_Operand];

	if (operand.m_Type == ILOperandType_Register)
	{
		if ((operand.m_Register.m_Base + 1) < ARRAY_SIZE(m_Entries) &&
			m_Entries[operand.m_Register.m_Base + 1])
		{
			return m_Entries[operand.m_Register.m_Base + 1]->GetEntry(instruction);
		}
	}

	if (operand.m_Type == ILOperandType_Value)
	{
		if ((operand.m_Value + 1) < ARRAY_SIZE(m_Entries) &&
			m_Entries[operand.m_Value + 1])
		{
			return m_Entries[operand.m_Value + 1]->GetEntry(instruction);
		}
	}

	if (!m_Entries[0])
	{
		return std::shared_ptr<AsmIndex>();
	}

	return m_Entries[0]->GetEntry(instruction);
}