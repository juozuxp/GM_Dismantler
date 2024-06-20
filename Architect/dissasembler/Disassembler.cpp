#include "Disassembler.hpp"
#include "Utility.hpp"

void Disassembler::Disassemble(const void* base, uint32_t size, std::vector<ILInstruction>& instructions) const
{
	ILInstruction instruction = {};

	for (uint32_t i = 0; i < size; i += instruction.m_Size)
	{
		instruction = Disassemble(reinterpret_cast<const uint8_t*>(base) + i);
		instructions.push_back(instruction);
	}
}

ILInstruction Disassembler::Disassemble(const uint8_t* instruction) const
{
	const uint8_t registerCount[] = { 16, 16, 8, 4, 8, 6, 16, 16 }; // general, xmm, mm, bnd, st, sreg, cr, dr
	const bool rexExtendable[] = { true, true, false, false, false, false, true, true }; // general, xmm, mm, bnd, st, sreg, cr, dr

	const Package* core = reinterpret_cast<const Package*>(CompiledPackage);
	const uint8_t* bytes = instruction;

	ILInstruction resolved = {};

	uint8_t mod = ~0;
	uint8_t reg = ~0;
	uint8_t mem = ~0;
	uint8_t disp = 0;

	Prefix prefixes = {};
	X0F383A x0F383A = X0F383A_None;

	const Package* base = core;
	const Package* package = &core[*bytes];
	while (true)
	{
		package = &base[*bytes];

		if (package->m_Type == PaType::FullRedirect)
		{
			base = &base[package->m_FullRedirect.m_BaseIndex];

			bytes++;
			if (*bytes == 0x38)
			{
				x0F383A = X0F383A_0F38;
				bytes++;
			}
			else if (*bytes == 0x3A)
			{
				x0F383A = X0F383A_0F3A;
				bytes++;
			}

			continue;
		}

		if (package->m_Type == PaType::Prefix)
		{
			if (prefixes.m_REX)
			{
				ILInstruction invalid = {};

				invalid.m_Size = 1;
				return invalid;
			}

			if (package->m_Prefix.m_Instruction != InsType_invalid)
			{
				if (prefixes.m_Instruction != InsType_invalid)
				{
					resolved.m_Type = package->m_Prefix.m_Instruction;
					resolved.m_Size = 1;
					return resolved;
				}

				prefixes.m_Instruction = package->m_Prefix.m_Instruction;
			}

			prefixes.m_Prefix |= package->m_Prefix.m_Prefix;
			bytes++;
			continue;
		}

		bytes++;
		while (package->m_Type == PaType::Redirect)
		{
			const Redirect& redirect = package->m_Redirect;

			if (redirect.m_Type == ReType::Prefix)
			{
				uint32_t value = redirect.m_Prefix.m_Value & ((Prefix_RexW << 1) - 1);
				uint8_t count = Utility::CountToBit(prefixes.m_Prefix & value);

				if (count == static_cast<uint8_t>(~0))
				{
					if (!redirect.m_Prefix.m_ValidDefault)
					{
						ILInstruction invalid = {};

						invalid.m_Size = 1;
						return invalid;
					}

					package = &core[redirect.m_BaseIndex + redirect.m_Prefix.m_IndexDefault];
				}
				else
				{
					uint8_t index = ((redirect.m_Prefix.m_Value >> 8) >> (count * 3)) & ((1 << 3) - 1);
					package = &core[redirect.m_BaseIndex + index];

					prefixes.m_Instruction = InsType_invalid;
				}
			}
			else if (redirect.m_Type == ReType::X0F383A)
			{
				if (((redirect.m_x0F383A.m_Value >> x0F383A) & 1) != 0)
				{
					uint8_t index = ((redirect.m_x0F383A.m_Value >> 4) >> (x0F383A * 2)) & ((1 << 2) - 1);
					package = &core[redirect.m_BaseIndex + index];

					x0F383A = X0F383A_None;
				}
				else
				{
					ILInstruction invalid = {};

					invalid.m_Size = 1;
					return invalid;
				}
			}
			else
			{
				if (mem == static_cast<uint8_t>(~0))
				{
					uint8_t modrrm = *bytes;

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
						ILInstruction invalid = {};

						invalid.m_Size = 1;
						return invalid;
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
						ILInstruction invalid = {};

						invalid.m_Size = 1;
						return invalid;
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
						ILInstruction invalid = {};

						invalid.m_Size = 1;
						return invalid;
					}
				} break;
				}

				package = &core[redirect.m_BaseIndex + index];
			}
		}

		break;
	}

	if (x0F383A != X0F383A_None)
	{
		ILInstruction invalid = {};

		invalid.m_Size = 1;
		return invalid;
	}

	if (prefixes.m_Instruction != InsType_invalid)
	{
		resolved.m_Size = 1;
		resolved.m_Type = prefixes.m_Instruction;
		return resolved;
	}

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
				uint8_t modrrm = *bytes;

				mem = (modrrm >> 0) & ((1 << 3) - 1);
				reg = (modrrm >> 3) & ((1 << 3) - 1);
				mod = (modrrm >> 6) & ((1 << 2) - 1);
			}

			if (mod == 3)
			{
				bool high = !prefixes.m_REX && (operand.m_Reg.m_Size == OpSize::base_8) && (operand.m_Register == Register::general) && mem >= 4;

				resolved.m_Operands[i].m_Type = ILOperandType_Register;

				resolved.m_Operands[i].m_Register.m_Base = mem - (high ? 4 : 0) + (rexExtendable[static_cast<uint8_t>(operand.m_Register)] ? base_extend : 0);
				resolved.m_Operands[i].m_Register.m_Type = operand.m_Register;

				resolved.m_Operands[i].m_Register.m_BaseHigh = high;

				if (resolved.m_Operands[i].m_Register.m_Base >= registerCount[static_cast<uint8_t>(operand.m_Register)])
				{
					ILInstruction invalid = {};

					invalid.m_Size = 1;
					return invalid;
				}

				break;
			}
			else
			{
				resolved.m_Operands[i].m_Type = ILOperandType_Memory;
				resolved.m_Operands[i].m_Memory.m_Segment = GetSegment(prefixes);
			}

			if (mem == 4) // rsp (sib)
			{
				constexpr ILMemoryScaler scaler[] = { ILMemoryScaler_1, ILMemoryScaler_2, ILMemoryScaler_4, ILMemoryScaler_8 };

				if (sib_base == static_cast<uint8_t>(~0))
				{
					uint8_t sib = *(bytes + 1);

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
							resolved.m_Operands[i].m_MemoryValue.m_Segment = GetSegment(prefixes);
							resolved.m_Operands[i].m_MemoryValue.m_Value = *reinterpret_cast<const uint32_t*>(bytes + 2);

							break;
						}
						else
						{
							resolved.m_Operands[i].m_Memory.m_Base = IL_INVALID_REGISTER;
							resolved.m_Operands[i].m_Memory.m_Offset = *reinterpret_cast<const int32_t*>(bytes + 2);
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

					resolved.m_Operands[i].m_Memory.m_Base = sib_base + base_extend;
					resolved.m_Operands[i].m_Memory.m_Offset = *reinterpret_cast<const int8_t*>(bytes + 2);
				}
				else if (mod == 2)
				{
					disp = 4;

					resolved.m_Operands[i].m_Memory.m_Base = sib_base + base_extend;
					resolved.m_Operands[i].m_Memory.m_Offset = *reinterpret_cast<const int32_t*>(bytes + 2);
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
				resolved.m_Operands[i].m_Relative.m_Segment = GetSegment(prefixes);
				resolved.m_Operands[i].m_Relative.m_Value = *reinterpret_cast<const uint32_t*>(bytes + 1);
				break;
			}
			else if (mod == 1)
			{
				disp = 1;
				resolved.m_Operands[i].m_Memory.m_Offset = *reinterpret_cast<const int8_t*>(bytes + 1);
			}
			else if (mod == 2)
			{
				disp = 4;
				resolved.m_Operands[i].m_Memory.m_Offset = *reinterpret_cast<const int32_t*>(bytes + 1);
			}

			resolved.m_Operands[i].m_Memory.m_Base = mem + base_extend;
			resolved.m_Operands[i].m_Memory.m_Index = IL_INVALID_REGISTER;
		} break;
		case OpType::reg:
		{
			resolved.m_Operands[i].m_Type = ILOperandType_Register;
			resolved.m_Operands[i].m_Register.m_Type = operand.m_Register;

			if (operand.m_Constant)
			{
				if (operand.m_Rex)
				{
					bool high = !prefixes.m_REX && (operand.m_Reg.m_Size == OpSize::base_8) && (operand.m_Register == Register::general) && operand.m_Value >= 4;

					resolved.m_Operands[i].m_Register.m_Base = operand.m_Value - (high ? 4 : 0) + base_extend;
					resolved.m_Operands[i].m_Register.m_BaseHigh = high;
				}
				else
				{
					resolved.m_Operands[i].m_Register.m_Base = operand.m_Value;
				}

				break;
			}

			if (mem == static_cast<uint8_t>(~0))
			{
				uint8_t modrrm = *bytes;

				mem = (modrrm >> 0) & ((1 << 3) - 1);
				reg = (modrrm >> 3) & ((1 << 3) - 1);
				mod = (modrrm >> 6) & ((1 << 2) - 1);
			}

			bool high = !prefixes.m_REX && (operand.m_Reg.m_Size == OpSize::base_8) && (operand.m_Register == Register::general) && reg >= 4;

			resolved.m_Operands[i].m_Register.m_Base = reg - (high ? 4 : 0) + (rexExtendable[static_cast<uint8_t>(operand.m_Register)] ? reg_extend : 0);
			resolved.m_Operands[i].m_Register.m_BaseHigh = high;

			if (resolved.m_Operands[i].m_Register.m_Base >= registerCount[static_cast<uint8_t>(operand.m_Register)])
			{
				ILInstruction invalid = {};

				invalid.m_Size = 1;
				return invalid;
			}
		} break;
		}
	}

	bytes += ((sib_base != static_cast<uint8_t>(~0)) ? 1 : 0) + ((mem != static_cast<uint8_t>(~0)) ? 1 : 0) + disp;

	for (uint8_t i = 0; i < ARRAY_SIZE(package->m_Instruction.m_Operands); i++)
	{
		const Operand& operand = package->m_Instruction.m_Operands[i];

		if (operand.m_Type == OpType::none)
		{
			break;
		}


		if (operand.m_Type != OpType::imm &&
			operand.m_Type != OpType::rel &&
			operand.m_Type != OpType::moffs)
		{
			continue;
		}

		int64_t value = 0;
		if (operand.m_Constant)
		{
			value = operand.m_Value;
		}
		else if (operand.m_Type == OpType::moffs)
		{
			value = *reinterpret_cast<const int64_t*>(bytes);
			bytes += 8;
		}
		else
		{
			switch (operand.m_Mem.m_Size)
			{
			case OpSize::base_8:
			{
				value = *reinterpret_cast<const int8_t*>(bytes);
				bytes += 1;
			} break;
			case OpSize::base_16:
			{
				if (operand.m_Mem.m_Override1 && prefixes.m_x66)
				{
					value = *reinterpret_cast<const int32_t*>(bytes);
					bytes += 4;
				}
				else if (operand.m_Mem.m_Override2 && prefixes.m_REXW)
				{
					value = *reinterpret_cast<const int64_t*>(bytes);
					bytes += 8;
				}
				else
				{
					value = *reinterpret_cast<const int16_t*>(bytes);
					bytes += 2;
				}
			} break;
			case OpSize::base_32:
			{
				if (operand.m_Mem.m_Override1 && prefixes.m_x66)
				{
					value = *reinterpret_cast<const int16_t*>(bytes);
					bytes += 2;
				}
				else if (operand.m_Mem.m_Override2 && prefixes.m_REXW)
				{
					value = *reinterpret_cast<const int64_t*>(bytes);
					bytes += 8;
				}
				else
				{
					value = *reinterpret_cast<const int32_t*>(bytes);
					bytes += 4;
				}
			} break;
			default:
			{
				value = *reinterpret_cast<const int64_t*>(bytes);
				bytes += 8;
			} break;
			}
		}

		if (operand.m_Type == OpType::imm)
		{
			resolved.m_Operands[i].m_Type = ILOperandType_Value;
			resolved.m_Operands[i].m_Value = value;
		}
		else if (operand.m_Type == OpType::rel)
		{
			resolved.m_Operands[i].m_Type = ILOperandType_ValueRelative;
			resolved.m_Operands[i].m_Relative.m_Value = value;
			resolved.m_Operands[i].m_Relative.m_Segment = IL_INVALID_REGISTER;
		}
		else if (operand.m_Type == OpType::moffs)
		{
			resolved.m_Operands[i].m_Type = ILOperandType_MemoryAbsolute;
			resolved.m_Operands[i].m_MemoryValue.m_Value = value;
			resolved.m_Operands[i].m_MemoryValue.m_Segment = GetSegment(prefixes);
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

	resolved.m_Size = bytes - instruction;
	resolved.m_Type = package->m_Instruction.m_Type;
	return resolved;
}

