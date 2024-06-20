#include "DescriptorOperand.hpp"
#include "tools/ArraySize.h"
#include <math.h>
#include <assert.h>

DescriptorOperand::DescriptorOperand(const std::vector<std::string>& variations)
{
	bool found = false;

	for (std::string variation : variations)
	{
		if (variation.empty())
		{
			continue;
		}

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

			if (ParseForSreg(variation))
			{
				continue;
			}

			if (ParseForCr(variation))
			{
				continue;
			}

			if (ParseForDr(variation))
			{
				continue;
			}

			ParseForGeneral(variation);
			continue;
		}

		switch (m_Type.m_Register)
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
		case Register::sreg:
		{
			ParseForSreg(variation);
		} break;
		case Register::cr:
		{
			ParseForCr(variation);
		} break;
		case Register::dr:
		{
			ParseForDr(variation);
		} break;
		}
	}

	assert(m_Size.m_Mem.m_Size != Size::base_64 || !m_Size.m_Mem.m_Override2);
	assert(m_Size.m_Reg.m_Size != Size::base_64 || !m_Size.m_Reg.m_Override2);

	assert(!(m_Size.m_Mem.m_Size == Size::base_512 || m_Size.m_Mem.m_Size == Size::base_256 || m_Size.m_Mem.m_Size == Size::base_80 || m_Size.m_Mem.m_Size == Size::base_8 || m_Size.m_Mem.m_Size == Size::undefined) || !(m_Size.m_Mem.m_Override1 && m_Size.m_Mem.m_Override2));
	assert(!(m_Size.m_Reg.m_Size == Size::base_512 || m_Size.m_Reg.m_Size == Size::base_256 || m_Size.m_Reg.m_Size == Size::base_80 || m_Size.m_Reg.m_Size == Size::base_8 || m_Size.m_Reg.m_Size == Size::undefined) || !(m_Size.m_Reg.m_Override1 && m_Size.m_Reg.m_Override2));
}

const DescriptorOperand::TypeMask& DescriptorOperand::GetTypeMask() const
{
	return m_Type;
}

const DescriptorOperand::SizeMask& DescriptorOperand::GetSizeMask() const
{
	return m_Size;
}

const DescriptorOperand::FlagMask& DescriptorOperand::GetFlagMask() const
{
	return m_Flags;
}

bool DescriptorOperand::ParseForMm(const std::string_view& variation)
{
	if (m_Type.m_Register != Register::mm)
	{
		if (variation.size() < 2)
		{
			return false;
		}

		if (strncmp(variation.data(), "mm", 2))
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

		m_Type.m_Register = Register::mm;
	}

	if (variation.size() >= 3)
	{
		if (!strncmp(variation.data(), "mm", 2))
		{
			if (variation[2] >= '0' && variation[2] <= '9')
			{
				m_Type.m_Type = Type::reg;
				AssignSize(&m_Size.m_Reg, 64);

				m_Flags.m_Constant = true;
				m_Flags.m_RegisterIndex = variation[2] - '0';
				return true;
			}
			else
			{
				if (variation.size() >= 4)
				{
					if (!strncmp(variation.data() + 2, "/m", 2))
					{
						m_Type.m_Type = Type::modrm;
						ParseSize(variation.data() + 4);

						return true;
					}
				}
			}
		}
	}
	else
	{
		assert(m_Type.m_Type == Type::none);

		m_Type.m_Type = Type::reg;
		AssignSize(&m_Size.m_Reg, 64);
	}

	return true;
}

bool DescriptorOperand::ParseForSt(const std::string_view& variation)
{
	if (m_Type.m_Register != Register::st)
	{
		if (variation.size() < 2)
		{
			return false;
		}

		if (strncmp(variation.data(), "st", 2))
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

		m_Type.m_Register = Register::st;
	}

	if (variation.size() >= 4)
	{
		if (!strncmp(variation.data(), "st", 2))
		{
			if (variation[2] == '(')
			{
				if (variation[3] >= '0' && variation[3] <= '9')
				{
					m_Type.m_Type = Type::reg;
					AssignSize(&m_Size.m_Reg, 80);

					m_Flags.m_Constant = true;
					m_Flags.m_RegisterIndex = variation[3] - '0';
					return true;
				}
			}
			else
			{
				if (!strncmp(variation.data() + 2, "/m", 2))
				{
					m_Type.m_Type = Type::modrm;
					ParseSize(variation.data() + 4);

					return true;
				}
			}
		}
	}
	else
	{
		assert(m_Type.m_Type == Type::none);

		m_Type.m_Type = Type::reg;
		AssignSize(&m_Size.m_Reg, 80);
	}

	return true;
}

bool DescriptorOperand::ParseForCr(const std::string_view& variation)
{
	if (m_Type.m_Register != Register::cr)
	{
		if (variation.size() < 2)
		{
			return false;
		}

		if (strncmp(variation.data(), "cr", 2))
		{
			return false;
		}

		m_Type.m_Type = Type::reg;
		m_Type.m_Register = Register::cr;
		AssignSize(&m_Size.m_Reg, 64);
	}

	return true;
}

bool DescriptorOperand::ParseForDr(const std::string_view& variation)
{
	if (m_Type.m_Register != Register::dr)
	{
		if (variation.size() < 2)
		{
			return false;
		}

		if (strncmp(variation.data(), "dr", 2))
		{
			return false;
		}

		m_Type.m_Type = Type::reg;
		m_Type.m_Register = Register::dr;
		AssignSize(&m_Size.m_Reg, 64);
	}

	return true;
}

bool DescriptorOperand::ParseForXmm(const std::string_view& variation)
{
	if (m_Type.m_Register != Register::xmm)
	{
		if (variation.size() < 3)
		{
			return false;
		}

		if (strncmp(variation.data(), "xmm", 3))
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

		m_Type.m_Register = Register::xmm;
	}

	if (variation.size() >= 4)
	{
		if (!strncmp(variation.data(), "xmm", 3))
		{
			if (variation[3] >= '0' && variation[3] <= '9')
			{
				m_Type.m_Type = Type::reg;
				AssignSize(&m_Size.m_Reg, 128);

				m_Flags.m_Constant = true;
				m_Flags.m_RegisterIndex = variation[3] - '0';
				return true;
			}
			else
			{
				if (variation.size() >= 5)
				{
					if (!strncmp(variation.data() + 3, "/m", 2))
					{
						m_Type.m_Type = Type::modrm;
						ParseSize(variation.data() + 5);

						return true;
					}
				}
			}
		}
	}
	else
	{
		assert(m_Type.m_Type == Type::none);

		m_Type.m_Type = Type::reg;
		AssignSize(&m_Size.m_Reg, 128);
	}

	return true;
}

bool DescriptorOperand::ParseForBnd(const std::string_view& variation)
{
	if (m_Type.m_Register != Register::bnd)
	{
		if (variation.size() < 3)
		{
			return false;
		}

		if (strncmp(variation.data(), "bnd", 3))
		{
			return false;
		}

		if (variation.size() != 3)
		{
			if (variation[3] != '/')
			{
				return false;
			}
		}

		m_Type.m_Register = Register::bnd;
	}

	if (variation.size() >= 5)
	{
		if (!strncmp(variation.data(), "bnd/m", 5))
		{
			m_Type.m_Type = Type::modrm;
			ParseSize(variation.data() + 5);

			return true;
		}
	}
	else
	{
		assert(m_Type.m_Type == Type::none);

		m_Type.m_Type = Type::reg;
		AssignSize(&m_Size.m_Reg, 128);
	}

	return true;
}

bool DescriptorOperand::ParseForSreg(const std::string_view& variation)
{
	constexpr const char* segments[] = { "es", "cs", "ss", "ds", "fs", "gs" };

	if (m_Type.m_Register != Register::sreg)
	{
		if (variation.size() == 2)
		{
			for (uint8_t i = 0; i < ARRAY_SIZE(segments); i++)
			{
				if (!strncmp(variation.data(), segments[i], 2))
				{
					m_Flags.m_Constant = true;
					m_Flags.m_RegisterIndex = i;

					break;
				}
			}

			if (!m_Flags.m_Constant)
			{
				return false;
			}
		}
		else
		{
			if (variation.size() < 4)
			{
				return false;
			}

			if (strncmp(variation.data(), "sreg", 3))
			{
				return false;
			}
		}

		m_Type.m_Type = Type::reg;
		m_Type.m_Register = Register::sreg;

		if (m_Flags.m_Constant && (m_Flags.m_RegisterIndex == static_cast<uint8_t>(Segments::fs) || m_Flags.m_RegisterIndex == static_cast<uint8_t>(Segments::gs)))
		{
			AssignSize(&m_Size.m_Reg, 64);
			AssignSize(&m_Size.m_Reg, 16);
		}
		else
		{
			AssignSize(&m_Size.m_Reg, 16);
		}
	}

	return true;
}

void DescriptorOperand::ParseForGeneral(const std::string_view& variation)
{
	constexpr const char* upperRegisters[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };
	constexpr const char* lowerRegisters[] = { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };

	if (variation[0] >= '0' && variation[0] <= '9')
	{
		if (m_Type.m_Type == Type::none)
		{
			m_Type.m_Type = Type::imm;

			m_Flags.m_Constant = true;
			m_Flags.m_RegisterIndex = atoi(variation.data());

			AssignSize(&m_Size.m_Mem, 8);
			return;
		}

		uint8_t offset = ParseSize(variation.data());
		if (variation.size() != offset)
		{
			if (variation[offset] == '/')
			{
				ParseForGeneral(variation.data() + offset + 1);
			}
		}
		return;
	}

	m_Type.m_Register = Register::general;

	uint8_t cursor = 0;
	if (variation[0] == 'r')
	{
		m_Type.m_Type = Type::reg;

		if (variation.size() == 1)
		{
			return;
		}

		if (variation[1] != '/' && variation[1] != 'e' && (variation[1] < '0' || variation[1] > '9'))
		{
			cursor = 1;
			AssignSize(&m_Size.m_Reg, 64);
		}
		else
		{
			if (variation.size() >= 3)
			{
				if (!strncmp(variation.data() + 1, "/m", 2))
				{
					m_Type.m_Type = Type::modrm;

					ParseSize(variation.data() + 3);
					return;
				}
				else if (!strncmp(variation.data() + 1, "el", 2))
				{
					m_Type.m_Type = Type::rel;
					
					ParseSize(variation.data() + 3);
					return;
				}
				else if (!strncmp(variation.data() + 1, "ex", 2))
				{
					m_Flags.m_Rex = true;
					return;
				}
			}

			uint8_t offset = ParseSize(variation.data() + 1) + 1;
			if (variation.size() != offset)
			{
				if (variation[offset] == '/')
				{
					ParseForGeneral(variation.data() + offset + 1);
				}
			}

			return;
		}
	}
	else
	{
		if (variation[0] == 'e')
		{
			cursor = 1;
			AssignSize(&m_Size.m_Reg, 32);
		}
		else
		{
			if (variation[0] == 'm')
			{
				if (m_Type.m_Type == Type::reg)
				{
					m_IsSeparateSize = true;
					m_Type.m_Type = Type::modrm;
				}
				else
				{
					m_Type.m_Type = Type::mem;
				}

				if (variation.size() == 1)
				{
					return;
				}

				if (variation[1] == 'o')
				{
					if (variation.size() < 5)
					{
						return;
					}

					if (strncmp(variation.data(), "moffs", 5))
					{
						return;
					}

					m_Type.m_Type = Type::moffs;

					ParseSize(variation.data() + 5);
					return;
				}

				ParseSize(variation.data() + 1);
				return;
			}
			else
			{
				if (variation.size() >= 3)
				{
					if (!strncmp(variation.data(), "imm", 3))
					{
						m_Type.m_Type = Type::imm;

						ParseSize(variation.data() + 3);
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
		if (!strncmp(variation.data() + cursor, upperRegisters[i], 2))
		{
			if (cursor == 0)
			{
				AssignSize(&m_Size.m_Reg, 16);
			}

			m_Type.m_Type = Type::reg;
			m_Flags.m_Constant = true;
			m_Flags.m_RegisterIndex = i;
			return;
		}
	}

	if (cursor != 0)
	{
		return;
	}

	for (uint8_t i = 0; i < ARRAY_SIZE(lowerRegisters); i++)
	{
		if (!strncmp(variation.data(), lowerRegisters[i], 2))
		{
			AssignSize(&m_Size.m_Reg, 8);
			m_Type.m_Type = Type::reg;

			m_Flags.m_Constant = true;
			m_Flags.m_RegisterIndex = i;
			return;
		}
	}
}

void DescriptorOperand::AssignSize(OperandSize* mask, uint32_t size)
{
	switch (size)
	{
	case 8:
	{
		assert(mask->m_Size == Size::base_8 || mask->m_Size == Size::undefined);
		mask->m_Size = Size::base_8;
	} break;
	case 16:
	{
		assert(mask->m_Size == Size::base_16 || mask->m_Size == Size::base_32 || mask->m_Size == Size::base_64 || mask->m_Size == Size::undefined);

		if (mask->m_Size == Size::base_32 || mask->m_Size == Size::base_64)
		{
			mask->m_Override1 = true;
		}
		else
		{
			mask->m_Size = Size::base_16;
		}
	} break;
	case 32:
	{
		assert(mask->m_Size == Size::base_16 || mask->m_Size == Size::base_32 || mask->m_Size == Size::undefined);

		if (mask->m_Size == Size::base_16)
		{
			mask->m_Override1 = true;
		}
		else
		{
			mask->m_Size = Size::base_32;
		}
	} break;
	case 64:
	{
		assert(mask->m_Size == Size::base_16 || mask->m_Size == Size::base_32 || mask->m_Size == Size::base_64 || mask->m_Size == Size::undefined);

		if (mask->m_Size == Size::base_32 || mask->m_Size == Size::base_64)
		{
			mask->m_Override2 = true;
		}
		else
		{
			mask->m_Size = Size::base_64;
		}
	} break;
	case 80:
	{
		assert(mask->m_Size == Size::base_80 || mask->m_Size == Size::undefined);
		mask->m_Size = Size::base_80;
	} break;
	case 128:
	{
		assert(mask->m_Size == Size::base_128 || mask->m_Size == Size::undefined);
		mask->m_Size = Size::base_128;
	} break;
	case 256:
	{
		assert(mask->m_Size == Size::base_128 || mask->m_Size == Size::base_256 || mask->m_Size == Size::undefined);

		if (mask->m_Size == Size::base_128)
		{
			mask->m_Override1 = true;
		}
		else
		{
			mask->m_Size = Size::base_256;
		}
	} break;
	case 512:
	{
		assert(mask->m_Size == Size::base_128 || mask->m_Size == Size::base_512 || mask->m_Size == Size::undefined);

		if (mask->m_Size == Size::base_128)
		{
			mask->m_Override2 = true;
		}
		else
		{
			mask->m_Size = Size::base_512;
		}
	} break;
	default:
	{
		assert(false);
	} break;
	}
}

uint8_t DescriptorOperand::ParseSize(const std::string_view& variation)
{
	if (variation.empty())
	{
		return 0;
	}

	uint32_t size = atoi(variation.data());
	if (m_Type.m_Type == Type::mem)
	{
		uint8_t offset = 0;

		if (size == 16 && variation.size() > 2 && variation[2] == ':')
		{
			offset = 3;

			size = atoi(variation.data() + 3);
			m_Size.m_Mem.m_Size = Size::far_32;
		}

		if (m_Size.m_Mem.m_Size == Size::far_32)
		{
			switch (size)
			{
			case 16:
			{
				m_Size.m_Mem.m_Override1 = true;
			} break;
			case 32:
			{
			} break;
			case 64:
			{
				m_Size.m_Mem.m_Override2 = true;
			} break;
			default:
			{
				assert(false);
			} break;
			}

			return (size == 0 ? 0 : floor(log10(size)) + 1) + offset;
		}
	}

	if (m_Type.m_Type == Type::reg || (m_Type.m_Type == Type::modrm && !m_IsSeparateSize))
	{
		AssignSize(&m_Size.m_Reg, size);
	}

	if (m_Type.m_Type != Type::reg)
	{
		AssignSize(&m_Size.m_Mem, size);
	}

	return size == 0 ? 0 : floor(log10(size)) + 1;
}