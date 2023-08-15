#include "AsmInstruction.hpp"
#include <assert.h>

AsmInstruction::AsmInstruction(const Index& index) :
	m_Template(index)
{
}

const AsmIndex::Index& AsmInstruction::GetTemplate() const
{
	return m_Template;
}

std::shared_ptr<const AsmIndex> AsmInstruction::GetEntry(const ILInstruction& instruction) const
{
	return shared_from_this();
}