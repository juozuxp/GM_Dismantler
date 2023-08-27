#pragma once
#include "AsmRedirect.hpp"

class AsmRedirectConstant : public AsmRedirect
{
public:
	AsmRedirectConstant(uint8_t operand);

public:
	void Set(uint8_t value, std::shared_ptr<AsmIndex> index); // if value is == ~0 it's considered default

public:
	std::shared_ptr<const class AsmInstruction> GetIndex(const ILInstruction& instruction) const override final;

private:
	std::shared_ptr<AsmIndex> m_Entries[17]; // indexed by register, where +8 is REX and 0 is default
};