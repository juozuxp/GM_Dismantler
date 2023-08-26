#include "AsmRedirectRegister.hpp"

AsmRedirectRegister::AsmRedirectRegister(uint8_t operand) :
	AsmRedirect(operand)
{
}

void AsmRedirectRegister::Set(Register regType, std::shared_ptr<AsmIndex> index)
{
	m_Entries[static_cast<uint8_t>(regType)] = index;
}

std::vector<uint8_t> AsmRedirectRegister::Assemble(const ILInstruction& instruction) const
{
	const ILOperand& operand = instruction.m_Operands[m_Operand];

	if (operand.m_Type != ILOperandType_Register ||
		!m_Entries[static_cast<uint8_t>(operand.m_Register.m_Type)])
	{
		return std::vector<uint8_t>();
	}

	return m_Entries[static_cast<uint8_t>(operand.m_Register.m_Type)]->Assemble(instruction);
}