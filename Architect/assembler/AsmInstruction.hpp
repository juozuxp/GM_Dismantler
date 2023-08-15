#pragma once
#include "AsmIndex.hpp"

class AsmInstruction : public AsmIndex
{
public:
	AsmInstruction(const Index& index);

public:
	const Index& GetTemplate() const;

public:
	std::shared_ptr<const AsmIndex> GetEntry(const ILInstruction& instruction) const override final;

private:
	Index m_Template;
};