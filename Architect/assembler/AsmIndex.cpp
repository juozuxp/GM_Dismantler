#include "AsmIndex.hpp"

AsmIndex::IndexOperand::IndexOperand(const Operand& operand)
{
	constexpr IndexOpType types[] = { IndexOpType::none, IndexOpType::reg, IndexOpType::modrm, IndexOpType::imm, IndexOpType::rel, IndexOpType::moffs };

	m_Type = types[static_cast<uint8_t>(operand.m_Type)];
	m_Register = operand.m_Register;

	m_Rex = operand.m_Rex;
	m_Constant = operand.m_Constant;
	m_RegisterIndex = operand.m_RegisterIndex;

	m_Reg = operand.m_Reg;
	m_Mem = operand.m_Mem;
}

AsmIndex::Index::Index(const Package& package)
{
	switch (package.m_Type)
	{
	case PaType::Instruction:
	{
		m_Type = package.m_Instruction.m_Type;

		uint8_t count = 0;
		for (const Operand& operand : package.m_Instruction.m_Operands)
		{
			if (operand.m_Type == OpType::none)
			{
				break;
			}

			m_Operands[count++] = IndexOperand(operand);
		}
	} break;
	case PaType::Prefix:
	{
		_ASSERT(package.m_Prefix.m_Instruction != InsType_invalid);
		m_Type = package.m_Prefix.m_Instruction;
	} break;
	}
}