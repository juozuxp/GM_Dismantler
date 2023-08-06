#pragma once
#include "AsmRedirect.hpp"

class AsmRedirectSize : public AsmRedirect
{
public:
	AsmRedirectSize(const Operand& operand, uint8_t operandIndex);

private:
	union
	{
		struct
		{
			uint16_t m_Undefined : 1;
			uint16_t m_Size8 : 1;
			uint16_t m_Size16 : 1;
			uint16_t m_Size32 : 1;
			uint16_t m_Size64 : 1;
			uint16_t m_Size80 : 1;
			uint16_t m_Size128 : 1;
			uint16_t m_Size256 : 1;
			uint16_t m_Size512 : 1;
		};

		uint16_t m_Value;
	} m_Valid;

	std::shared_ptr<AsmIndex> m_Table[9]; // for each valid entry
};

