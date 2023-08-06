#pragma once
#include "AsmIndex.hpp"

class AsmRedirect : public AsmIndex
{
public:
	AsmRedirect(uint8_t operand);

protected:
	uint8_t m_Operand;
};