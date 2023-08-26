#pragma once
#include "AsmRedirect.hpp"

class AsmRedirectRegister : public AsmRedirect
{
public:
	AsmRedirectRegister(uint8_t operand);

public:
	void Set(Register regType, std::shared_ptr<AsmIndex> index);

public:
	std::vector<uint8_t> Assemble(const ILInstruction& instruction) const override final;

private:
	std::shared_ptr<AsmIndex> m_Entries[8];
};