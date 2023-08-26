#pragma once
#include "AsmIndex.hpp"

class AsmInstruction : public AsmIndex
{
public:
	enum class DispImmSize
	{
		none,
		size8,
		size16,
		size32,
		size64
	};

public:
	AsmInstruction(const Index& index);

public:
	const Index& GetTemplate() const;

public:
	std::vector<uint8_t> Assemble(const ILInstruction& instruction) const override final;

private:
	Index m_Template;
};