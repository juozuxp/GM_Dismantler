#include "AsmInstruction.hpp"
#include "Utility.hpp"
#include <assert.h>

AsmInstruction::AsmInstruction(const Index& index) :
	m_Template(index)
{
}

const AsmIndex::Index& AsmInstruction::GetTemplate() const
{
	return m_Template;
}

std::vector<uint8_t> AsmInstruction::Assemble(const ILInstruction& instruction) const
{
	constexpr PfxType segmentToPrefix[] = { Prefix_CS, Prefix_SS, Prefix_DS, Prefix_ES, Prefix_GS, Prefix_FS };
	constexpr uint8_t prefixToByte[] = { 0x9B, 0xF3, 0xF2, 0x66, 0x48, 0x40, 0x41, 0x42, 0x44, 0xF0, 0x67, 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65 };

	Prefix prefixes = {};

	switch (m_Template.m_Prefix)
	{
	case 0x66:
	{
		prefixes.m_x66 = true;
	} break;
	case 0x48:
	{
		prefixes.m_REXW = true;
	} break;
	case 0xF2:
	{
		prefixes.m_REPNZ = true;
	} break;
	case 0xF3:
	{
		prefixes.m_REPZ = true;
	} break;
	}

	bool sib_valid = false;
	bool modrm_valid = false;

	DispImmSize imm_type = DispImmSize::none;
	DispImmSize disp_type = DispImmSize::none;

	int64_t imm = 0;
	int64_t disp = 0;

	uint8_t modrm_mem = 0;
	uint8_t modrm_reg = 0;
	uint8_t modrm_mod = 0;

	uint8_t sib_base = 0;
	uint8_t sib_index = 0;
	uint8_t sib_scale = 0;

	for (uint8_t i = 0; i < ARRAY_SIZE(instruction.m_Operands); i++)
	{
		const ILOperand& operand = instruction.m_Operands[i];
		const IndexOperand& reference = m_Template.m_Operands[i];

		if (operand.m_Type == ILOperandType_None)
		{
			if (reference.m_Type != IndexOpType::none)
			{
				return std::vector<uint8_t>();
			}

			break;
		}

		const OperandSize& size = reference.m_Size[static_cast<uint8_t>(operand.m_Type == ILOperandType_Register ? IndexOpSize::reg : IndexOpSize::mem)];
		switch (operand.m_Scale)
		{
		case ILOperandScale_Undefined:
		{
			if (size.m_Size == OpSize::undefined)
			{
				break;
			}

			return std::vector<uint8_t>();
		} break;
		case ILOperandScale_8:
		{
			if (size.m_Size == OpSize::base_8)
			{
				break;
			}

			return std::vector<uint8_t>();
		} break;
		case ILOperandScale_16:
		{
			if (size.m_Size == OpSize::base_16)
			{
				break;
			}

			if (size.m_Size == OpSize::base_32 &&
				size.m_Override1)
			{
				prefixes.m_x66 = true;
				break;
			}
			else if (size.m_Size == OpSize::base_64 &&
				size.m_Override1)
			{
				prefixes.m_x66 = true;
				break;
			}

			return std::vector<uint8_t>();
		} break;
		case ILOperandScale_32:
		{
			if (size.m_Size == OpSize::base_32)
			{
				break;
			}

			if (size.m_Size == OpSize::base_16 &&
				size.m_Override1)
			{
				prefixes.m_x66 = true;
				break;
			}

			return std::vector<uint8_t>();
		} break;
		case ILOperandScale_64:
		{
			if (size.m_Size == OpSize::base_64)
			{
				break;
			}

			if (size.m_Size == OpSize::base_16 &&
				size.m_Override2)
			{
				prefixes.m_REXW = true;
				break;
			}
			else if (size.m_Size == OpSize::base_32 &&
				size.m_Override2)
			{
				prefixes.m_REXW = true;
				break;
			}

			return std::vector<uint8_t>();
		} break;
		case ILOperandScale_128:
		{
			if (size.m_Size == OpSize::base_128)
			{
				break;
			}

			return std::vector<uint8_t>();
		} break;
		case ILOperandScale_256:
		{
			if (size.m_Size == OpSize::base_256)
			{
				break;
			}

			return std::vector<uint8_t>();
		} break;
		case ILOperandScale_512:
		{
			if (size.m_Size == OpSize::base_512)
			{
				break;
			}

			return std::vector<uint8_t>();
		} break;
		}
	}

	for (uint8_t i = 0; i < ARRAY_SIZE(instruction.m_Operands); i++)
	{
		const ILOperand& operand = instruction.m_Operands[i];
		const IndexOperand& reference = m_Template.m_Operands[i];

		if (operand.m_Type == ILOperandType_None)
		{
			break;
		}

		switch (operand.m_Type)
		{
		case ILOperandType_Memory:
		{
			if (reference.m_Type != IndexOpType::mem &&
				reference.m_Type != IndexOpType::modrm)
			{
				return std::vector<uint8_t>();
			}

			modrm_valid = true;
			if (operand.m_Memory.m_Segment != IL_INVALID_REGISTER)
			{
				prefixes.m_Prefix |= segmentToPrefix[operand.m_Memory.m_Segment];
			}

			if ((operand.m_Memory.m_Index != IL_INVALID_REGISTER &&
				operand.m_Memory.m_Base != IL_INVALID_REGISTER) ||
				(operand.m_Memory.m_Index != IL_INVALID_REGISTER &&
				operand.m_Memory.m_Base == IL_INVALID_REGISTER &&
				operand.m_Memory.m_Scale != ILMemoryScaler_1)) // [reg + reg], or [reg * x + offs] (where x != 1) required
			{
				modrm_mem = 4;

				sib_valid = true;
				if (operand.m_Memory.m_Index >= 8)
				{
					prefixes.m_REXX = true;
				}

				sib_index = operand.m_Memory.m_Index & (8 - 1);
				if (operand.m_Memory.m_Base != IL_INVALID_REGISTER)
				{
					if (operand.m_Memory.m_Base >= 8)
					{
						prefixes.m_REXB = true;
					}

					sib_base = operand.m_Memory.m_Base & (8 - 1);
				}
				else
				{
					sib_base = 5;
				}

				sib_scale = operand.m_Memory.m_Scale;
			}
			else
			{
				uint8_t reg = operand.m_Memory.m_Base != IL_INVALID_REGISTER ? operand.m_Memory.m_Base : operand.m_Memory.m_Index;
				if (reg >= 8)
				{
					prefixes.m_REXB = true;
				}

				modrm_mem = reg & (8 - 1);
			}

			if (sib_valid && sib_base == 5)
			{
				disp_type = DispImmSize::size32;
				disp = operand.m_Memory.m_Offset;
			}
			else
			{
				if (operand.m_Memory.m_Offset != 0)
				{
					if (operand.m_Memory.m_Offset <= INT8_MAX &&
						operand.m_Memory.m_Offset >= INT8_MIN)
					{
						modrm_mod = 1;
						disp_type = DispImmSize::size8;
					}
					else
					{
						modrm_mod = 2;
						disp_type = DispImmSize::size32;
					}

					disp = operand.m_Memory.m_Offset;
				}
			}
		} break;
		case ILOperandType_Register:
		{
			if (reference.m_Type != IndexOpType::reg &&
				reference.m_Type != IndexOpType::modrm)
			{
				return std::vector<uint8_t>();
			}

			if (reference.m_Register != operand.m_Register.m_Type)
			{
				return std::vector<uint8_t>();
			}

			uint8_t reg = operand.m_Register.m_Base + (operand.m_Register.m_BaseHigh ? 4 : 0);
			if (reference.m_Constant)
			{
				if (reference.m_Rex &&
					reference.m_Value == (reg & (8 - 1)))
				{
					if (reg >= 8)
					{
						prefixes.m_REXB = true;
					}

					if (operand.m_Scale == ILOperandScale_8 && reg >= 4)
					{
						prefixes.m_REX = true;
					}

					break;
				}
				else if (reference.m_Value == reg)
				{
					break;
				}

				return std::vector<uint8_t>();
			}

			modrm_valid = true;
			if (operand.m_Scale == ILOperandScale_8 && reg >= 4)
			{
				prefixes.m_REX = true;
			}

			if (reference.m_Type == IndexOpType::modrm)
			{
				if (reg >= 8)
				{
					prefixes.m_REXB = true;
				}

				modrm_mod = 3;
				modrm_mem = reg & (8 - 1);
			}
			else
			{
				if (reg >= 8)
				{
					prefixes.m_REXR = true;
				}

				modrm_reg = reg & (8 - 1);
			}
		} break;
		case ILOperandType_MemoryRelative:
		{
			if (reference.m_Type != IndexOpType::mem &&
				reference.m_Type != IndexOpType::modrm)
			{
				return std::vector<uint8_t>();
			}

			if (operand.m_MemoryValue.m_Segment != IL_INVALID_REGISTER)
			{
				prefixes.m_Prefix |= segmentToPrefix[operand.m_MemoryValue.m_Segment];
			}

			modrm_valid = true;

			modrm_mod = 0;
			modrm_mem = 5;
			
			disp = operand.m_MemoryValue.m_Value;
		} break;
		case ILOperandType_MemoryAbsolute:
		{
			if (reference.m_Type != IndexOpType::mem &&
				reference.m_Type != IndexOpType::modrm &&
				reference.m_Type != IndexOpType::moffs)
			{
				return std::vector<uint8_t>();
			}

			if (operand.m_MemoryValue.m_Value >= INT32_MAX &&
				operand.m_MemoryValue.m_Value <= INT32_MIN &&
				reference.m_Type != IndexOpType::moffs)
			{
				return std::vector<uint8_t>();
			}

			if (operand.m_MemoryValue.m_Segment != IL_INVALID_REGISTER)
			{
				prefixes.m_Prefix |= segmentToPrefix[operand.m_MemoryValue.m_Segment];
			}

			if (reference.m_Type == IndexOpType::moffs)
			{
				disp_type = DispImmSize::size64;
				disp = operand.m_MemoryValue.m_Value;
			}
			else
			{
				sib_valid = true;
				modrm_valid = true;

				modrm_mod = 0;
				modrm_mem = 4;

				sib_index = 4;
				sib_base = 5;
				
				disp_type = DispImmSize::size32;
				disp = operand.m_MemoryValue.m_Value;
			}
		} break;
		case ILOperandType_Value:
		{
			if (reference.m_Type != IndexOpType::imm)
			{
				return std::vector<uint8_t>();
			}

			if (imm_type != DispImmSize::none)
			{
				disp = imm;
				disp_type = imm_type;
			}

			imm = operand.m_Value;
			switch (operand.m_Scale)
			{
			case ILOperandScale_8:
			{
				imm_type = DispImmSize::size8;
			} break;
			case ILOperandScale_16:
			{
				imm_type = DispImmSize::size16;
			} break;
			case ILOperandScale_32:
			{
				imm_type = DispImmSize::size32;
			} break;
			case ILOperandScale_64:
			{
				imm_type = DispImmSize::size64;
			} break;
			}
		} break;
		case ILOperandType_ValueRelative:
		{
			if (reference.m_Type != IndexOpType::rel)
			{
				return std::vector<uint8_t>();
			}

			disp = operand.m_Value;
			switch (operand.m_Scale)
			{
			case ILOperandScale_8:
			{
				disp_type = DispImmSize::size8;
			} break;
			case ILOperandScale_16:
			{
				disp_type = DispImmSize::size16;
			} break;
			case ILOperandScale_32:
			{
				disp_type = DispImmSize::size32;
			} break;
			case ILOperandScale_64:
			{
				disp_type = DispImmSize::size64;
			} break;
			}
		} break;
		}
	}

	std::vector<uint8_t> bytes;

	uint8_t rex = 0;
	uint16_t prefix = prefixes.m_Prefix;
	for (uint8_t i = 0; prefix != 0; i++, prefix >>= 1)
	{
		if (prefix & 1)
		{
			if (prefixToByte[i] >= 0x40 && 
				prefixToByte[i] <= 0x4F)
			{
				rex |= prefixToByte[i];
				continue;
			}

			bytes.push_back(prefixToByte[i]);
		}
	}

	if (rex != 0)
	{
		bytes.push_back(rex);
	}

	if (m_Template.m_HasBase)
	{
		bytes.push_back(m_Template.m_Base);
	}

	if (m_Template.m_HasX0F383A)
	{
		bytes.push_back(m_Template.m_X0F383A);
	}

	bytes.push_back(m_Template.m_Instruction);

	if (m_Template.m_HasModRM)
	{
		bytes.push_back(m_Template.m_ModRM);
	}
	else if (m_Template.m_OnlyReg || modrm_valid)
	{
		uint8_t modrm = 0;

		if (modrm_valid)
		{
			modrm = (modrm_mem << 0) | (modrm_mod << 6);
		}

		if (m_Template.m_OnlyReg)
		{
			modrm |= m_Template.m_ModRM << 3;
		}
		else
		{
			modrm |= modrm_reg << 3;
		}

		bytes.push_back(modrm);
	}

	if (sib_valid)
	{
		bytes.push_back((sib_base << 0) | (sib_index << 3) | (sib_scale << 6));
	}

	if (disp_type != DispImmSize::none)
	{
		uint32_t offset = bytes.size();

		switch (disp_type)
		{
		case DispImmSize::size8:
		{
			bytes.resize(bytes.size() + sizeof(uint8_t));
			*reinterpret_cast<uint8_t*>(bytes.data() + offset) = static_cast<uint8_t>(disp);
		} break;
		case DispImmSize::size16:
		{
			bytes.resize(bytes.size() + sizeof(uint16_t));
			*reinterpret_cast<uint16_t*>(bytes.data() + offset) = static_cast<uint16_t>(disp);
		} break;
		case DispImmSize::size32:
		{
			bytes.resize(bytes.size() + sizeof(uint32_t));
			*reinterpret_cast<uint32_t*>(bytes.data() + offset) = static_cast<uint32_t>(disp);
		} break;
		case DispImmSize::size64:
		{
			bytes.resize(bytes.size() + sizeof(uint64_t));
			*reinterpret_cast<uint64_t*>(bytes.data() + offset) = static_cast<uint64_t>(disp);
		} break;
		}
	}

	if (imm_type != DispImmSize::none)
	{
		uint32_t offset = bytes.size();

		switch (imm_type)
		{
		case DispImmSize::size8:
		{
			bytes.resize(bytes.size() + sizeof(uint8_t));
			*reinterpret_cast<uint8_t*>(bytes.data() + offset) = static_cast<uint8_t>(imm);
		} break;
		case DispImmSize::size16:
		{
			bytes.resize(bytes.size() + sizeof(uint16_t));
			*reinterpret_cast<uint16_t*>(bytes.data() + offset) = static_cast<uint16_t>(imm);
		} break;
		case DispImmSize::size32:
		{
			bytes.resize(bytes.size() + sizeof(uint32_t));
			*reinterpret_cast<uint32_t*>(bytes.data() + offset) = static_cast<uint32_t>(imm);
		} break;
		case DispImmSize::size64:
		{
			bytes.resize(bytes.size() + sizeof(uint64_t));
			*reinterpret_cast<uint64_t*>(bytes.data() + offset) = static_cast<uint64_t>(imm);
		} break;
		}
	}

	return bytes;
}