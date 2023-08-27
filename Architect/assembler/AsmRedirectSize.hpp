#pragma once
#include "AsmRedirect.hpp"

class AsmRedirectSize : public AsmRedirect
{
public:
	AsmRedirectSize(uint8_t operand);

public:
	void Set(OpSize size, std::shared_ptr<AsmIndex> entry);

public:
	std::shared_ptr<const class AsmInstruction> GetIndex(const ILInstruction& instruction) const override final;

private:
	std::shared_ptr<AsmIndex> m_Entries[10]; // indexed from OpSize

private:
	static constexpr OpSize c_ILToOpSizeMap[] = { OpSize::undefined, OpSize::base_8, OpSize::base_16, OpSize::base_32, OpSize::base_64, OpSize::base_80, OpSize::base_128, OpSize::base_256, OpSize::base_512 };
};