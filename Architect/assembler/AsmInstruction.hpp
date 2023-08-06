#pragma once
#include "AsmIndex.hpp"

class AsmInstruction : public AsmIndex
{
public:
	std::shared_ptr<AsmIndex> GetEntry(const ILInstruction& instruction) override final;
};