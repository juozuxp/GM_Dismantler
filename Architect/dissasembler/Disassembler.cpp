#include "Disassembler.hpp"
#include "ArraySize.hpp"

std::vector<ILInstruction> Disassembler::Disassemble(const void* base, uint32_t size)
{
	std::vector<ILInstruction> instructions;

	instructions.push_back(Disassemble(reinterpret_cast<const uint8_t*>(base)));

	return instructions;
}

ILInstruction Disassembler::Disassemble(const uint8_t* instruction)
{
	const Package* core = reinterpret_cast<const Package*>(CompiledPackage);

	uint8_t mod = ~0;
	uint8_t reg = ~0;
	uint8_t mem = ~0;
	uint8_t disp = 0;

	Prefix prefixes = {};
	X0F383A x0F383A = {};

	const Package* base = core;
	const Package* package = &core[*instruction];
	while (true)
	{
		package = &base[*instruction];

		if (package->m_Type == PaType::FullRedirect)
		{
			base = &base[package->m_FullRedirect.m_BaseIndex];

			instruction++;
			if (*instruction == 0x38)
			{
				x0F383A = X0F383A_0F38;
				instruction++;
			}
			else if (*instruction == 0x3A)
			{
				x0F383A = X0F383A_0F3A;
				instruction++;
			}

			continue;
		}

		if (package->m_Type == PaType::Prefix)
		{
			if (prefixes.m_REX)
			{
				return ILInstruction();
			}

			if (package->m_Prefix.m_Instruction != InsType_invalid)
			{
				if (prefixes.m_Instruction != InsType_invalid)
				{
					ILInstruction instruction;

					instruction.m_Type = prefixes.m_Instruction;
					return instruction;
				}

				prefixes.m_Instruction = package->m_Prefix.m_Instruction;
			}

			prefixes.m_Prefix |= package->m_Prefix.m_Prefix;
			instruction++;
			continue;
		}

		instruction++;
		while (package->m_Type == PaType::Redirect)
		{
			const Redirect& redirect = package->m_Redirect;

			if (redirect.m_Type == ReType::Prefix)
			{
				uint32_t value = redirect.m_Prefix.m_Value & ((Prefix_x66 << 1) - 1);
				uint8_t count = CountToBit(prefixes.m_Prefix & value);

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
					uint8_t modrrm = *instruction;

					mem = (modrrm >> 0) & ((1 << 3) - 1);
					reg = (modrrm >> 3) & ((1 << 3) - 1);
					mod = (modrrm >> 6) & ((1 << 2) - 1);
				}

				uint8_t index = 0;

				switch (redirect.m_Type)
				{
				case ReType::Mem:
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
				case ReType::Reg:
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
				case ReType::Mod:
				{
					if (((redirect.m_Mod.m_Value >> mod) & 1) != 0)
					{
						index = ((redirect.m_Mod.m_Value >> 4) >> (mod * 2)) & ((1 << 2) - 1);
					}
					else
					{
						return ILInstruction();
					}
				} break;
				case ReType::X0F383A:
				{
					if (((redirect.m_x0F383A.m_Value >> x0F383A) & 1) != 0)
					{
						index = ((redirect.m_x0F383A.m_Value >> 4) >> (x0F383A * 2)) & ((1 << 2) - 1);
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

	ILInstruction resolved = {};

	uint8_t sib_base = ~0;
	uint8_t sib_scale = ~0;
	uint8_t sib_index = ~0;

	uint8_t reg_extend = prefixes.m_REXR ? 8 : 0;
	uint8_t base_extend = prefixes.m_REXB ? 8 : 0;
	uint8_t index_extend = prefixes.m_REXX ? 8 : 0;

	for (uint8_t i = 0; i < ARRAY_SIZE(package->m_Instruction.m_Operands); i++)
	{
		const Operand& operand = package->m_Instruction.m_Operands[i];

		if (operand.m_Type == OpType::none)
		{
			break;
		}

		switch (operand.m_Type)
		{
		case OpType::modrm:
		{
			if (mem == static_cast<uint8_t>(~0))
			{
				uint8_t modrrm = *instruction;

				mem = (modrrm >> 0) & ((1 << 3) - 1);
				reg = (modrrm >> 3) & ((1 << 3) - 1);
				mod = (modrrm >> 6) & ((1 << 2) - 1);
			}

			if (mod == 3)
			{
				resolved.m_Operands[i].m_Type = ILOperandType_Register;

				resolved.m_Operands[i].m_Register.m_Base = mem + base_extend;
				resolved.m_Operands[i].m_Register.m_Type = operand.m_Register;

				resolved.m_Operands[i].m_Register.m_BaseHigh = !prefixes.m_REX && (operand.m_Reg.m_Size == OpSize::base_8) && mem >= 4;

				break;
			}
			else
			{
				resolved.m_Operands[i].m_Type = ILOperandType_Memory;
			}

			if (mem == 4) // rsp (sib)
			{
				constexpr ILMemoryScaler scaler[] = { ILMemoryScaler_1, ILMemoryScaler_2, ILMemoryScaler_4, ILMemoryScaler_8 };

				if (sib_base == static_cast<uint8_t>(~0))
				{
					uint8_t sib = *(instruction + 1);

					sib_base = (sib >> 0) & ((1 << 3) - 1);
					sib_index = (sib >> 3) & ((1 << 3) - 1);
					sib_scale = (sib >> 6) & ((1 << 2) - 1);
				}
				
				if (mod == 0)
				{
					if (sib_base == 5)
					{
						disp = 4;

						if (sib_index == 4)
						{
							resolved.m_Operands[i].m_Type = ILOperandType_MemoryAbsolute;
							resolved.m_Operands[i].m_Value = *reinterpret_cast<const uint32_t*>(instruction + 2);

							break;
						}
						else
						{
							resolved.m_Operands[i].m_Memory.m_Base = IL_INVALID_REGISTER;
							resolved.m_Operands[i].m_Memory.m_Offset = *reinterpret_cast<const int32_t*>(instruction + 2);
						}
					}
					else
					{
						resolved.m_Operands[i].m_Memory.m_Base = sib_base + base_extend;
					}
				}
				else if (mod == 1)
				{
					disp = 1;
					resolved.m_Operands[i].m_Memory.m_Offset = *reinterpret_cast<const int8_t*>(instruction + 2);
				}
				else if (mod == 2)
				{
					disp = 4;
					resolved.m_Operands[i].m_Memory.m_Offset = *reinterpret_cast<const int8_t*>(instruction + 2);
				}

				if (sib_index == 4)
				{
					resolved.m_Operands[i].m_Memory.m_Index = IL_INVALID_REGISTER;
				}
				else
				{
					resolved.m_Operands[i].m_Memory.m_Index = sib_index + index_extend;
				}

				resolved.m_Operands[i].m_Memory.m_Scale = scaler[sib_scale];
				break;
			}
			else if (mem == 5 && mod == 0)
			{
				disp = 4;

				resolved.m_Operands[i].m_Type = ILOperandType_MemoryRelative;
				resolved.m_Operands[i].m_Value = *reinterpret_cast<const uint32_t*>(instruction + 1);
				break;
			}
			else if (mod == 1)
			{
				disp = 1;
				resolved.m_Operands[i].m_Memory.m_Offset = *reinterpret_cast<const int8_t*>(instruction + 1);
			}
			else if (mod == 2)
			{
				disp = 4;
				resolved.m_Operands[i].m_Memory.m_Offset = *reinterpret_cast<const int32_t*>(instruction + 1);
			}

			resolved.m_Operands[i].m_Memory.m_Base = mem + base_extend;
			resolved.m_Operands[i].m_Memory.m_Index = IL_INVALID_REGISTER;
		} break;
		case OpType::reg:
		{
			resolved.m_Operands[i].m_Type = ILOperandType_Register;

			if (operand.m_Constant)
			{
				resolved.m_Operands[i].m_Register.m_Type = operand.m_Register;

				if (operand.m_Rex)
				{
					resolved.m_Operands[i].m_Register.m_Base = operand.m_RegisterIndex + reg_extend;
					resolved.m_Operands[i].m_Register.m_BaseHigh = !prefixes.m_REX && (operand.m_Reg.m_Size == OpSize::base_8) && operand.m_RegisterIndex >= 4;
				}
				else
				{
					resolved.m_Operands[i].m_Register.m_Base = operand.m_RegisterIndex;
				}

				break;
			}

			if (mem == static_cast<uint8_t>(~0))
			{
				uint8_t modrrm = *instruction;

				mem = (modrrm >> 0) & ((1 << 3) - 1);
				reg = (modrrm >> 3) & ((1 << 3) - 1);
				mod = (modrrm >> 6) & ((1 << 2) - 1);
			}

			resolved.m_Operands[i].m_Register.m_Base = reg + reg_extend;
			resolved.m_Operands[i].m_Register.m_BaseHigh = !prefixes.m_REX && (operand.m_Reg.m_Size == OpSize::base_8) && reg >= 4;
		} break;
		}
	}

	instruction += ((sib_base != static_cast<uint8_t>(~0)) ? 1 : 0) + ((mem != static_cast<uint8_t>(~0)) ? 1 : 0) + disp;

	for (uint8_t i = 0; i < ARRAY_SIZE(package->m_Instruction.m_Operands); i++)
	{
		const Operand& operand = package->m_Instruction.m_Operands[i];

		if (operand.m_Type == OpType::none)
		{
			break;
		}

		if (operand.m_Type == OpType::imm)
		{
			resolved.m_Operands[i].m_Type = ILOperandType_Value;
		}
		else if (operand.m_Type == OpType::rel)
		{
			resolved.m_Operands[i].m_Type = ILOperandType_ValueRelative;
		}
		else
		{
			continue;
		}

		switch (operand.m_Mem.m_Size)
		{
		case OpSize::base_8:
		{
			resolved.m_Operands[i].m_Value = *reinterpret_cast<const uint8_t*>(instruction);
			instruction += 1;
		} break;
		case OpSize::base_16:
		{
			if (operand.m_Mem.m_Override1 && prefixes.m_x66)
			{
				resolved.m_Operands[i].m_Value = *reinterpret_cast<const uint32_t*>(instruction);
				instruction += 4;
			}
			else if (operand.m_Mem.m_Override2 && prefixes.m_REXW)
			{
				resolved.m_Operands[i].m_Value = *reinterpret_cast<const uint64_t*>(instruction);
				instruction += 8;
			}
			else
			{
				resolved.m_Operands[i].m_Value = *reinterpret_cast<const uint16_t*>(instruction);
				instruction += 2;
			}
		} break;
		case OpSize::base_32:
		{
			if (operand.m_Mem.m_Override1 && prefixes.m_x66)
			{
				resolved.m_Operands[i].m_Value = *reinterpret_cast<const uint16_t*>(instruction);
				instruction += 2;
			}
			else if (operand.m_Mem.m_Override2 && prefixes.m_REXW)
			{
				resolved.m_Operands[i].m_Value = *reinterpret_cast<const uint64_t*>(instruction);
				instruction += 8;
			}
			else
			{
				resolved.m_Operands[i].m_Value = *reinterpret_cast<const uint32_t*>(instruction);
				instruction += 4;
			}
		} break;
		default:
		{
			resolved.m_Operands[i].m_Value = *reinterpret_cast<const uint64_t*>(instruction);
			instruction += 8;
		} break;
		}
	}

	for (uint8_t i = 0; i < ARRAY_SIZE(package->m_Instruction.m_Operands); i++)
	{
		const Operand& operand = package->m_Instruction.m_Operands[i];

		if (operand.m_Type == OpType::none)
		{
			break;
		}

		const OperandSize* size = &operand.m_Mem;
		if (operand.m_Type == OpType::modrm)
		{
			if (mod == 3)
			{
				size = &operand.m_Reg;
			}
		}
		else if (operand.m_Type == OpType::reg)
		{
			size = &operand.m_Reg;
		}

		switch (size->m_Size)
		{
		case OpSize::base_8:
		{
			resolved.m_Operands[i].m_Scale = ILOperandScale_8;
		} break;
		case OpSize::base_16:
		{
			if (size->m_Override1 && prefixes.m_x66)
			{
				resolved.m_Operands[i].m_Scale = ILOperandScale_32;
			}
			else if (size->m_Override2 && prefixes.m_REXW)
			{
				resolved.m_Operands[i].m_Scale = ILOperandScale_64;
			}
			else
			{
				resolved.m_Operands[i].m_Scale = ILOperandScale_16;
			}
		} break;
		case OpSize::base_32:
		{
			if (size->m_Override1 && prefixes.m_x66)
			{
				resolved.m_Operands[i].m_Scale = ILOperandScale_16;
			}
			else if (size->m_Override2 && prefixes.m_REXW)
			{
				resolved.m_Operands[i].m_Scale = ILOperandScale_64;
			}
			else
			{
				resolved.m_Operands[i].m_Scale = ILOperandScale_32;
			}
		} break;
		case OpSize::base_64:
		{
			resolved.m_Operands[i].m_Scale = ILOperandScale_64;
		} break;
		case OpSize::base_80:
		{
			resolved.m_Operands[i].m_Scale = ILOperandScale_80;
		} break;
		case OpSize::base_128:
		{
			resolved.m_Operands[i].m_Scale = ILOperandScale_128;
		} break;
		case OpSize::base_256:
		{
			resolved.m_Operands[i].m_Scale = ILOperandScale_256;
		} break;
		case OpSize::base_512:
		{
			resolved.m_Operands[i].m_Scale = ILOperandScale_512;
		} break;
		}
	}

	resolved.m_Type = package->m_Instruction.m_Type;
	return resolved;
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