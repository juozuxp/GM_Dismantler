#include "AsmInstruction.hpp"
#include "Utility.hpp"
#include <assert.h>

AsmInstruction::AsmInstruction(const Index& index) :
	m_Template(index)
{
}

AsmInstruction::Blob::Blob(const Index& index)
{
	m_Valid = true;

	m_HasBase = index.m_HasBase;
	m_Base = index.m_Base;

	m_HasX0F383A = index.m_HasX0F383A;
	m_X0F383A = index.m_X0F383A;

	m_Core = index.m_Instruction;

	switch (index.m_Prefix)
	{
	case 0x66:
	{
		m_Prefixes.m_x66 = true;
	} break;
	case 0x48:
	{
		m_Prefixes.m_REXW = true;
	} break;
	case 0xF2:
	{
		m_Prefixes.m_REPNZ = true;
	} break;
	case 0xF3:
	{
		m_Prefixes.m_REPZ = true;
	} break;
	}

	if (index.m_OnlyReg)
	{
		m_ModRMValid = true;
		
		m_ModRM.m_Reg = index.m_ModRM;
	}
	else if (index.m_HasModRM)
	{
		m_ModRMValid = true;

		m_ModRM.m_Mem = (index.m_ModRM >> 0) & (8 - 1);
		m_ModRM.m_Reg = (index.m_ModRM >> 3) & (8 - 1);
		m_ModRM.m_Mod = (index.m_ModRM >> 6) & (4 - 1);
	}
}

uint8_t AsmInstruction::Blob::GetSize() const
{
	constexpr uint8_t prefixToByte[] = { 0x9B, 0xF3, 0xF2, 0x66, 0x48, 0x40, 0x41, 0x42, 0x44, 0xF0, 0x67, 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65 };
	constexpr uint8_t dispImmToSize[] = { 0, 1, 2, 4, 8 };

	if (!m_Valid)
	{
		return 0;
	}

	uint8_t size = 1;

	bool rex = false;
	uint16_t prefix = m_Prefixes.m_Prefix;
	for (uint8_t i = 0; prefix != 0; i++, prefix >>= 1)
	{
		if (prefix & 1)
		{
			if (prefixToByte[i] >= 0x40 &&
				prefixToByte[i] <= 0x4F)
			{
				rex = true;
				continue;
			}

			size += 1;
		}
	}

	if (rex)
	{
		size += 1;
	}

	if (m_HasBase)
	{
		size += 1;
	}

	if (m_HasX0F383A)
	{
		size += 1;
	}

	if (m_ModRMValid)
	{
		size += 1;
	}

	if (m_SIBValid)
	{
		size += 1;
	}

	size += dispImmToSize[static_cast<uint8_t>(m_DispSize)];
	size += dispImmToSize[static_cast<uint8_t>(m_ImmSize)];

	return size;
}

void AsmInstruction::Compile(const std::vector<Blob>& blobs, std::vector<uint8_t>& bytes)
{
	constexpr uint8_t prefixToByte[] = { 0x9B, 0xF3, 0xF2, 0x66, 0x48, 0x40, 0x41, 0x42, 0x44, 0xF0, 0x67, 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65 };

	uint32_t size = 0;
	for (const Blob& blob : blobs)
	{
		if (!blob.m_Valid)
		{
			continue;
		}

		size += blob.GetSize();

		uint8_t rex = 0;
		uint16_t prefix = blob.m_Prefixes.m_Prefix;
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

		if (blob.m_HasBase)
		{
			bytes.push_back(blob.m_Base);
		}

		if (blob.m_HasX0F383A)
		{
			bytes.push_back(blob.m_X0F383A);
		}

		bytes.push_back(blob.m_Core);
		if (blob.m_ModRMValid)
		{
			bytes.push_back((blob.m_ModRM.m_Mem << 0) | (blob.m_ModRM.m_Reg << 3) | (blob.m_ModRM.m_Mod << 6));
		}

		if (blob.m_SIBValid)
		{
			bytes.push_back((blob.m_SIB.m_Base << 0) | (blob.m_SIB.m_Index << 3) | (blob.m_SIB.m_Scale << 6));
		}

		if (blob.m_DispSize != DispImmSize::none)
		{
			uint32_t offset = bytes.size();

			switch (blob.m_DispSize)
			{
			case DispImmSize::size8:
			{
				bytes.resize(offset + sizeof(uint8_t));
				*reinterpret_cast<uint8_t*>(bytes.data() + offset) = static_cast<uint8_t>(blob.m_Disp);
			} break;
			case DispImmSize::size16:
			{
				bytes.resize(offset + sizeof(uint16_t));
				*reinterpret_cast<uint16_t*>(bytes.data() + offset) = static_cast<uint16_t>(blob.m_Disp);
			} break;
			case DispImmSize::size32:
			{
				bytes.resize(offset + sizeof(uint32_t));
				*reinterpret_cast<uint32_t*>(bytes.data() + offset) = static_cast<uint32_t>(blob.m_Disp);
			} break;
			case DispImmSize::size64:
			{
				bytes.resize(offset + sizeof(uint64_t));
				*reinterpret_cast<uint64_t*>(bytes.data() + offset) = static_cast<uint64_t>(blob.m_Disp);
			} break;
			}
		}

		if (blob.m_ImmSize != DispImmSize::none)
		{
			uint32_t offset = bytes.size();

			switch (blob.m_ImmSize)
			{
			case DispImmSize::size8:
			{
				bytes.resize(offset + sizeof(uint8_t));
				*reinterpret_cast<uint8_t*>(bytes.data() + offset) = static_cast<uint8_t>(blob.m_Imm);
			} break;
			case DispImmSize::size16:
			{
				bytes.resize(offset + sizeof(uint16_t));
				*reinterpret_cast<uint16_t*>(bytes.data() + offset) = static_cast<uint16_t>(blob.m_Imm);
			} break;
			case DispImmSize::size32:
			{
				bytes.resize(offset + sizeof(uint32_t));
				*reinterpret_cast<uint32_t*>(bytes.data() + offset) = static_cast<uint32_t>(blob.m_Imm);
			} break;
			case DispImmSize::size64:
			{
				bytes.resize(offset + sizeof(uint64_t));
				*reinterpret_cast<uint64_t*>(bytes.data() + offset) = static_cast<uint64_t>(blob.m_Imm);
			} break;
			}
		}
	}
}

AsmInstruction::Blob AsmInstruction::Assemble(const ILInstruction& instruction) const
{
	constexpr uint8_t prefixToByte[] = { 0x9B, 0xF3, 0xF2, 0x66, 0x48, 0x40, 0x41, 0x42, 0x44, 0xF0, 0x67, 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65 };
	constexpr PfxType segmentToPrefix[] = { Prefix_CS, Prefix_SS, Prefix_DS, Prefix_ES, Prefix_GS, Prefix_FS };

	Blob blob = Blob(m_Template);

	for (uint8_t i = 0; i < ARRAY_SIZE(instruction.m_Operands); i++)
	{
		const ILOperand& operand = instruction.m_Operands[i];
		const IndexOperand& reference = m_Template.m_Operands[i];

		if (operand.m_Type == ILOperandType_None)
		{
			if (reference.m_Type != IndexOpType::none)
			{
				return Blob();
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

			return Blob();
		} break;
		case ILOperandScale_8:
		{
			if (size.m_Size == OpSize::base_8)
			{
				break;
			}

			return Blob();
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
				blob.m_Prefixes.m_x66 = true;
				break;
			}
			else if (size.m_Size == OpSize::base_64 &&
				size.m_Override1)
			{
				blob.m_Prefixes.m_x66 = true;
				break;
			}

			return Blob();
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
				blob.m_Prefixes.m_x66 = true;
				break;
			}

			return Blob();
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
				blob.m_Prefixes.m_REXW = true;
				break;
			}
			else if (size.m_Size == OpSize::base_32 &&
				size.m_Override2)
			{
				blob.m_Prefixes.m_REXW = true;
				break;
			}

			return Blob();
		} break;
		case ILOperandScale_128:
		{
			if (size.m_Size == OpSize::base_128)
			{
				break;
			}

			return Blob();
		} break;
		case ILOperandScale_256:
		{
			if (size.m_Size == OpSize::base_256)
			{
				break;
			}

			return Blob();
		} break;
		case ILOperandScale_512:
		{
			if (size.m_Size == OpSize::base_512)
			{
				break;
			}

			return Blob();
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
				return Blob();
			}

			blob.m_ModRMValid = true;
			if (operand.m_Memory.m_Segment != IL_INVALID_REGISTER)
			{
				blob.m_Prefixes.m_Prefix |= segmentToPrefix[operand.m_Memory.m_Segment];
			}

			if ((operand.m_Memory.m_Index != IL_INVALID_REGISTER &&
				operand.m_Memory.m_Base != IL_INVALID_REGISTER) ||
				(operand.m_Memory.m_Index != IL_INVALID_REGISTER &&
				operand.m_Memory.m_Base == IL_INVALID_REGISTER &&
				operand.m_Memory.m_Scale != ILMemoryScaler_1)) // [reg + reg], or [reg * x + offs] (where x != 1) required
			{
				blob.m_ModRM.m_Mem = 4;

				blob.m_SIBValid = true;
				if (operand.m_Memory.m_Index >= 8)
				{
					blob.m_Prefixes.m_REXX = true;
				}

				blob.m_SIB.m_Index = operand.m_Memory.m_Index & (8 - 1);
				if (operand.m_Memory.m_Base != IL_INVALID_REGISTER)
				{
					if (operand.m_Memory.m_Base >= 8)
					{
						blob.m_Prefixes.m_REXB = true;
					}

					blob.m_SIB.m_Base = operand.m_Memory.m_Base & (8 - 1);
				}
				else
				{
					blob.m_ModRM.m_Mod = 0;
					blob.m_SIB.m_Base = 5;
				}

				blob.m_SIB.m_Scale = operand.m_Memory.m_Scale;
			}
			else
			{
				uint8_t reg = operand.m_Memory.m_Base != IL_INVALID_REGISTER ? operand.m_Memory.m_Base : operand.m_Memory.m_Index;
				if (reg >= 8)
				{
					blob.m_Prefixes.m_REXB = true;
				}

				blob.m_ModRM.m_Mem = reg & (8 - 1);
			}

			if (blob.m_SIBValid && blob.m_SIB.m_Base == 5)
			{
				blob.m_DispSize = DispImmSize::size32;
				blob.m_Disp = operand.m_Memory.m_Offset;
			}
			else
			{
				if (operand.m_Memory.m_Offset != 0)
				{
					if (operand.m_Memory.m_Offset <= INT8_MAX &&
						operand.m_Memory.m_Offset >= INT8_MIN)
					{
						blob.m_ModRM.m_Mod = 1;
						blob.m_DispSize = DispImmSize::size8;
					}
					else
					{
						blob.m_ModRM.m_Mod = 2;
						blob.m_DispSize = DispImmSize::size32;
					}

					blob.m_Disp = operand.m_Memory.m_Offset;
				}
			}
		} break;
		case ILOperandType_Register:
		{
			if (reference.m_Type != IndexOpType::reg &&
				reference.m_Type != IndexOpType::modrm)
			{
				return Blob();
			}

			if (reference.m_Register != operand.m_Register.m_Type)
			{
				return Blob();
			}

			uint8_t reg = operand.m_Register.m_Base + (operand.m_Register.m_BaseHigh ? 4 : 0);
			if (reference.m_Constant)
			{
				if (reference.m_Rex &&
					reference.m_Value == (reg & (8 - 1)))
				{
					if (reg >= 8)
					{
						blob.m_Prefixes.m_REXB = true;
					}

					if (operand.m_Scale == ILOperandScale_8 && reg >= 4)
					{
						blob.m_Prefixes.m_REX = true;
					}

					break;
				}
				else if (reference.m_Value == reg)
				{
					break;
				}

				return Blob();
			}

			blob.m_ModRMValid = true;
			if (operand.m_Scale == ILOperandScale_8 && reg >= 4)
			{
				blob.m_Prefixes.m_REX = true;
			}

			if (reference.m_Type == IndexOpType::modrm)
			{
				if (reg >= 8)
				{
					blob.m_Prefixes.m_REXB = true;
				}

				blob.m_ModRM.m_Mod = 3;
				blob.m_ModRM.m_Mem = reg & (8 - 1);
			}
			else
			{
				if (reg >= 8)
				{
					blob.m_Prefixes.m_REXR = true;
				}

				blob.m_ModRM.m_Reg = reg & (8 - 1);
			}
		} break;
		case ILOperandType_MemoryRelative:
		{
			if (reference.m_Type != IndexOpType::mem &&
				reference.m_Type != IndexOpType::modrm)
			{
				return Blob();
			}

			if (operand.m_Relative.m_Segment != IL_INVALID_REGISTER)
			{
				blob.m_Prefixes.m_Prefix |= segmentToPrefix[operand.m_Relative.m_Segment];
			}

			blob.m_ModRMValid = true;

			blob.m_ModRM.m_Mod = 0;
			blob.m_ModRM.m_Mem = 5;

			blob.m_DispSize = DispImmSize::size32;
			blob.m_Disp = operand.m_MemoryValue.m_Value;
		} break;
		case ILOperandType_MemoryAbsolute:
		{
			if (reference.m_Type != IndexOpType::mem &&
				reference.m_Type != IndexOpType::modrm &&
				reference.m_Type != IndexOpType::moffs)
			{
				return Blob();
			}

			if (operand.m_MemoryValue.m_Value >= INT32_MAX &&
				operand.m_MemoryValue.m_Value <= INT32_MIN &&
				reference.m_Type != IndexOpType::moffs)
			{
				return Blob();
			}

			if (operand.m_MemoryValue.m_Segment != IL_INVALID_REGISTER)
			{
				blob.m_Prefixes.m_Prefix |= segmentToPrefix[operand.m_MemoryValue.m_Segment];
			}

			if (reference.m_Type == IndexOpType::moffs)
			{
				blob.m_DispSize = DispImmSize::size64;
				blob.m_Disp = operand.m_MemoryValue.m_Value;
			}
			else
			{
				blob.m_SIBValid = true;
				blob.m_ModRMValid = true;

				blob.m_ModRM.m_Mod = 0;
				blob.m_ModRM.m_Mem = 4;

				blob.m_SIB.m_Index = 4;
				blob.m_SIB.m_Base = 5;
				
				blob.m_DispSize = DispImmSize::size32;
				blob.m_Disp = operand.m_MemoryValue.m_Value;
			}
		} break;
		case ILOperandType_Value:
		{
			if (reference.m_Type != IndexOpType::imm)
			{
				return Blob();
			}

			if (blob.m_ImmSize != DispImmSize::none)
			{
				blob.m_Disp = blob.m_Imm;
				blob.m_DispSize = blob.m_ImmSize;
			}

			blob.m_Imm = operand.m_Value;
			switch (operand.m_Scale)
			{
			case ILOperandScale_8:
			{
				blob.m_ImmSize = DispImmSize::size8;
			} break;
			case ILOperandScale_16:
			{
				blob.m_ImmSize = DispImmSize::size16;
			} break;
			case ILOperandScale_32:
			{
				blob.m_ImmSize = DispImmSize::size32;
			} break;
			case ILOperandScale_64:
			{
				blob.m_ImmSize = DispImmSize::size64;
			} break;
			}
		} break;
		case ILOperandType_ValueRelative:
		{
			if (reference.m_Type != IndexOpType::rel)
			{
				return Blob();
			}

			blob.m_Disp = operand.m_Relative.m_Value;

			switch (operand.m_Scale)
			{
			case ILOperandScale_8:
			{
				blob.m_DispSize = DispImmSize::size8;
			} break;
			case ILOperandScale_16:
			{
				blob.m_DispSize = DispImmSize::size16;
			} break;
			case ILOperandScale_32:
			{
				blob.m_DispSize = DispImmSize::size32;
			} break;
			case ILOperandScale_64:
			{
				blob.m_DispSize = DispImmSize::size64;
			} break;
			}
		} break;
		}
	}


	return blob;
}

std::shared_ptr<const AsmInstruction> AsmInstruction::GetIndex(const ILInstruction& instruction) const
{
	return std::reinterpret_pointer_cast<const AsmInstruction>(shared_from_this());
}