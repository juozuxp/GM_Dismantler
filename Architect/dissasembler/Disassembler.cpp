#include "Disassembler.hpp"

std::vector<ILInstruction> Disassembler::Disassemble(const void* base, uint32_t size)
{
	std::vector<ILInstruction> instructions;

	instructions.push_back(Disassemble(reinterpret_cast<const uint8_t*>(base)));

	return instructions;
}

ILInstruction Disassembler::Disassemble(const uint8_t* instruction)
{
	const Package* base = reinterpret_cast<const Package*>(Architecture);
	const Package* core = base;

	PrefixPackage prefixes = {};
	while (true)
	{
		const Package* package = &base[*instruction];

		if (package->m_Type == Package_FullRedirection)
		{
			base = &base[package->m_FullRedirection.m_BaseIndex];
			instruction++;
			continue;
		}

		if (package->m_Type == Package_Prefix)
		{
			prefixes.m_Value |= package->m_Prefix.m_Value;
			instruction++;
			continue;
		}

		uint8_t mod = ~0;
		uint8_t reg = ~0;
		uint8_t mem = ~0;

		while (package->m_Type == Package_Redirection)
		{
			const RedirectionPackage& redirect = package->m_Redirection;

			if (redirect.m_Type == Redirect_Prefix)
			{
				uint32_t value = redirect.m_Prefix.m_Value & ((Prefix_x0F3A << 1) - 1);
				uint8_t count = CountToBit(prefixes.m_Value & value);

				if (count == static_cast<uint8_t>(~0))
				{
					if (redirect.m_Prefix.m_ValidDefault)
					{
						return ILInstruction();
					}

					package = &core[redirect.m_BaseIndex + redirect.m_Prefix.m_IndexDefault];
				}
				else if (((redirect.m_Prefix.m_Value >> count) & 1) != 0)
				{
					uint8_t index = ((redirect.m_Prefix.m_Value >> 8) >> (count * 3)) & ((1 << 3) - 1);

					package = &core[redirect.m_BaseIndex + index];
				}
				else
				{
					return ILInstruction();
				}
			}
			else
			{
				if (mem == static_cast<uint8_t>(~0))
				{
					uint8_t modrrm = *(instruction + 1);

					mem = (modrrm >> 0) & ((1 << 3) - 1);
					reg = (modrrm >> 3) & ((1 << 3) - 1);
					mod = (modrrm >> 6) & ((1 << 2) - 1);
				}

				uint8_t index = 0;

				switch (redirect.m_Type)
				{
				case Redirect_Mem:
				{
					if (((redirect.m_RegRM.m_Value >> mem) & 1) != 0)
					{
						index = ((redirect.m_RegRM.m_Value >> 8) >> (mem * 3)) & ((1 << 3) - 1);
					}
					else
					{
						return ILInstruction();
					}
				} break;
				case Redirect_Reg:
				{
					if (((redirect.m_RegRM.m_Value >> reg) & 1) != 0)
					{
						index = ((redirect.m_RegRM.m_Value >> 8) >> (reg * 3)) & ((1 << 3) - 1);
					}
					else
					{
						return ILInstruction();
					}
				} break;
				case Redirect_Mod:
				{
					if (((redirect.m_Mod.m_Value >> mod) & 1) != 0)
					{
						index = ((redirect.m_Mod.m_Value >> 8) >> (mod * 2)) & ((1 << 2) - 1);
					}
					else
					{
						return ILInstruction();
					}
				} break;
				}

				package = &core[redirect.m_BaseIndex + index];
			}
		}

		break;
	}

	return ILInstruction();
}

uint8_t Disassembler::CountToBit(uint32_t value)
{
	if (value == 0)
	{
		return ~0;
	}

	for (uint8_t i = 0;; i++, value >>= 1)
	{
		if (value & 1)
		{
			return i;
		}
	}

	return 0;
}