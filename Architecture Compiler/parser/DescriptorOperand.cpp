#include "DescriptorOperand.hpp"
#include "tools/ArraySize.h"

DescriptorOperand::DescriptorOperand(const std::vector<std::string>& variations)
{
	bool found = false;

	for (std::string variation : variations)
	{
		if (!found)
		{
			found = true;

			if (ParseForXmm(variation))
			{
				continue;
			}

			if (ParseForMm(variation))
			{
				continue;
			}

			if (ParseForSt(variation))
			{
				continue;
			}

			if (ParseForBnd(variation))
			{
				continue;
			}

			ParseForGeneral(variation);
			continue;
		}

		switch (m_Register)
		{
		case Register::general:
		{
			ParseForGeneral(variation);
		} break;
		case Register::xmm:
		{
			ParseForXmm(variation);
		} break;
		case Register::mm:
		{
			ParseForMm(variation);
		} break;
		case Register::bnd:
		{
			ParseForBnd(variation);
		} break;
		case Register::st:
		{
			ParseForSt(variation);
		} break;
		}
	}
}

bool DescriptorOperand::ParseForMm(const std::string& variation)
{
	if (m_Register != Register::mm)
	{
		if (variation.size() < 2)
		{
			return false;
		}

		if (strncmp(variation.c_str(), "mm", 2))
		{
			return false;
		}

		if (variation.size() != 2)
		{
			if (variation[2] != '/' && (variation[2] < '0' || variation[2] > '9'))
			{
				return false;
			}
		}

		m_Size64 = true;
		m_Type = Type::reg;
		m_Register = Register::mm;
	}

	if (variation.size() >= 3)
	{
		if (!strncmp(variation.c_str(), "mm", 2))
		{
			if (variation[2] >= '0' && variation[2] <= '9')
			{
				m_Constant = true;
				m_RegisterIndex = variation[2] - '0';
				return true;
			}
			else
			{
				if (variation.size() >= 4)
				{
					if (!strncmp(variation.c_str() + 2, "/m", 2))
					{
						ParseSize(variation.c_str() + 4);

						m_Type = Type::modrm;
						return true;
					}
				}
			}
		}
	}

	return true;
}

bool DescriptorOperand::ParseForSt(const std::string& variation)
{
	if (m_Register != Register::st)
	{
		if (variation.size() < 2)
		{
			return false;
		}

		if (strncmp(variation.c_str(), "st", 2))
		{
			return false;
		}

		if (variation.size() != 2)
		{
			if (variation[2] != '(' && variation[2] != '/')
			{
				return false;
			}
		}

		m_Type = Type::reg;
		m_Register = Register::st;
	}

	if (variation.size() >= 4)
	{
		if (!strncmp(variation.c_str(), "st", 2))
		{
			if (variation[2] == '(')
			{
				if (variation[3] >= '0' && variation[3] <= '9')
				{
					m_Constant = true;
					m_RegisterIndex = variation[3] - '0';
					return true;
				}
			}
			else
			{
				if (!strncmp(variation.c_str() + 2, "/m", 2))
				{
					ParseSize(variation.c_str() + 4);

					m_Type = Type::modrm;
					return true;
				}
			}
		}
	}

	return true;
}

bool DescriptorOperand::ParseForXmm(const std::string& variation)
{
	if (m_Register != Register::xmm)
	{
		if (variation.size() < 3)
		{
			return false;
		}

		if (strncmp(variation.c_str(), "xmm", 3))
		{
			return false;
		}

		if (variation.size() != 3)
		{
			if (variation[3] != '/' && (variation[3] < '0' || variation[3] > '9'))
			{
				return false;
			}
		}

		m_Size128 = true;
		m_Type = Type::reg;
		m_Register = Register::xmm;
	}

	if (variation.size() >= 4)
	{
		if (!strncmp(variation.c_str(), "xmm", 3))
		{
			if (variation[3] >= '0' && variation[3] <= '9')
			{
				m_Constant = true;
				m_RegisterIndex = variation[3] - '0';
				return true;
			}
			else
			{
				if (variation.size() >= 5)
				{
					if (!strncmp(variation.c_str() + 3, "/m", 2))
					{
						ParseSize(variation.c_str() + 5);

						m_Type = Type::modrm;
						return true;
					}
				}
			}
		}
	}

	return true;
}

bool DescriptorOperand::ParseForBnd(const std::string& variation)
{
	if (m_Register != Register::bnd)
	{
		if (variation.size() < 3)
		{
			return false;
		}

		if (strncmp(variation.c_str(), "bnd", 3))
		{
			return false;
		}

		if (variation.size() != 3)
		{
			if (variation[3] != '/' && (variation[3] < '0' || variation[3] > '9'))
			{
				return false;
			}
		}

		m_Type = Type::reg;
		m_Register = Register::bnd;
	}

	if (variation.size() >= 5)
	{
		if (!strncmp(variation.c_str(), "bnd/m", 5))
		{
			ParseSize(variation.c_str() + 5);

			m_Type = Type::modrm;
			return true;
		}
	}

	return true;
}

void DescriptorOperand::ParseForGeneral(const std::string& variation)
{
	constexpr const char* upperRegisters[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
	constexpr const char* lowerRegisters[] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };

	if (variation[0] >= '0' && variation[0] <= '9')
	{
		ParseSize(variation.c_str());
		return;
	}

	m_Register = Register::general;

	uint8_t cursor = 0;
	if (variation[0] == 'r')
	{
		m_Type = Type::reg;

		if (variation.size() == 1)
		{
			return;
		}

		if (variation[1] != '/' && (variation[1] < '0' || variation[1] > '9'))
		{
			cursor = 1;

			m_Size64 = true;
		}
		else
		{
			if (variation.size() >= 3)
			{
				if (!strncmp(variation.c_str() + 1, "/m", 2))
				{
					m_Type = Type::modrm;

					ParseSize(variation.c_str() + 3);
					return;
				}
			}

			ParseSize(variation.c_str() + 1);
			return;
		}
	}
	else
	{
		if (variation[0] == 'e')
		{
			m_Size32 = true;
		}
		else
		{
			if (variation[0] == 'm')
			{
				m_Type = Type::mem;

				if (variation.size() == 1)
				{
					return;
				}

				ParseSize(variation.c_str() + 1);
			}
			else
			{
				if (variation.size() >= 3)
				{
					if (!strncmp(variation.c_str(), "imm", 3))
					{
						m_Type = Type::imm;

						ParseSize(variation.c_str() + 3);
						return;
					}
				}
			}
		}
	}

	if (variation.size() - cursor < 2)
	{
		return;
	}

	for (uint8_t i = 0; i < ARRAY_SIZE(upperRegisters); i++)
	{
		if (!strncmp(variation.c_str() + cursor, upperRegisters[i], 2))
		{
			if (cursor == 0)
			{
				m_Size16 = true;
			}

			m_Constant = true;
			m_RegisterIndex = i;
			return;
		}
	}

	if (cursor != 0)
	{
		return;
	}

	for (uint8_t i = 0; i < ARRAY_SIZE(lowerRegisters); i++)
	{
		if (!strncmp(variation.c_str(), lowerRegisters[i], 2))
		{
			m_Size8 = true;
			m_Constant = true;
			m_RegisterIndex = i;
			return;
		}
	}
}

void DescriptorOperand::ParseSize(const std::string_view& variation)
{
	if (variation.empty())
	{
		return;
	}

	switch (atoi(variation.data()))
	{
	case 8:
	{
		m_Size8 = true;
	} break;
	case 16:
	{
		m_Size16 = true;
	} break;
	case 32:
	{
		m_Size32 = true;
	} break;
	case 64:
	{
		m_Size64 = true;
	} break;
	case 128:
	{
		m_Size128 = true;
	} break;
	case 256:
	{
		m_Size256 = true;
	} break;
	case 512:
	{
		m_Size512 = true;
	} break;
	}
}