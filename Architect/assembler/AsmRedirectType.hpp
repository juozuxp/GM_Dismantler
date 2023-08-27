#pragma once
#include "AsmRedirect.hpp"

class AsmRedirectType : public AsmRedirect
{
public:
	enum class Type
	{
		none,
		reg,
		mem,
		imm,
		rel,
		moffs
	};

public:
	AsmRedirectType(uint8_t operand);

public:
	void Set(IndexOpType type, std::shared_ptr<AsmIndex> entry);

public:
	std::shared_ptr<const class AsmInstruction> GetIndex(const ILInstruction& instruction) const override final;

private:
	std::shared_ptr<AsmIndex> m_Entries[6] = {};

private:
	static constexpr Type c_IndexTypeToType[] = { Type::none, Type::reg, Type::mem, Type::none, Type::imm, Type::rel, Type::moffs };
	static constexpr Type c_ILTypeToType[] = { Type::none, Type::mem, Type::reg, Type::imm, Type::rel, Type::mem, Type::moffs };
};