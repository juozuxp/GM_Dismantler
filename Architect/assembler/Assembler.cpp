#include <unordered_map>
#include "Assembler.hpp"

#include "AsmInstruction.hpp"
#include "AsmIndex.hpp"
#include "Utility.hpp"

bool Assembler::s_ReverseTableValid = false;
std::shared_ptr<AsmIndex> Assembler::s_ReverseTable[InsType_ARRAY_MAX];

Assembler::Instruction::Instruction(const Package* package) :
	m_Package(package)
{
}

Assembler::Assembler()
{
	if (!s_ReverseTableValid)
	{
		std::vector<AsmIndex::Index>* types = new std::vector<AsmIndex::Index>[InsType_ARRAY_MAX]; // warning thrown otherwise, this many vectors on the stack is a no go

		std::vector<uint8_t> base = std::vector<uint8_t>();
		std::vector<AsmIndex::Index> indexes = FlattenInstructions(reinterpret_cast<const Package*>(CompiledPackage), reinterpret_cast<const Package*>(CompiledPackage), base);

		for (const AsmIndex::Index& index : indexes)
		{
			types[index.m_Type].push_back(index);
		}

		for (uint32_t i = 0; i < InsType_ARRAY_MAX; i++)
		{
			s_ReverseTable[i] = AsmIndex::MapTree(types[i]);
		}

		delete[] types;
	}
}

std::vector<uint8_t> Assembler::Assemble(const std::vector<ILInstruction>& instructions) const
{
	std::vector<AsmInstruction::Blob> blobs;

	blobs.reserve(instructions.size());
	for (const ILInstruction& instruction : instructions)
	{
		std::shared_ptr<const AsmInstruction> index = s_ReverseTable[instruction.m_Type]->GetIndex(instruction);
		if (!index)
		{
			continue;
		}

		blobs.push_back(index->Assemble(instruction));
	}

	std::vector<uint8_t> bytes;

	AsmInstruction::Compile(blobs, bytes);

	return bytes;
}

Assembler::Level::Level(ReType type, uint32_t index) :
	m_Type(type), m_Index(index)
{
}

std::vector<AsmIndex::Index> Assembler::FlattenInstructions(const Package* base, const Package* package, std::vector<uint8_t>& baseBytes)
{
	std::vector<AsmIndex::Index> indexes;
	std::vector<Instruction> instructions;
	for (uint8_t i = 0; i < 255; i++, package++)
	{
		if (package->m_Type == PaType::Invalid)
		{
			continue;
		}

		if (package->m_Type == PaType::FullRedirect)
		{
			baseBytes.push_back(i);

			std::vector<AsmIndex::Index> result = FlattenInstructions(base, base + package->m_FullRedirect.m_BaseIndex, baseBytes);

			baseBytes.pop_back();

			indexes.insert(indexes.end(), result.begin(), result.end());
		}

		uint32_t cursor = instructions.size();

		FlattenChain(instructions, base, package);

		for (uint32_t j = cursor; j < instructions.size(); j++)
		{
			instructions[j].m_Byte = i;
		}
	}

	std::vector<AsmIndex::Index> result = IndexInstructions(instructions, baseBytes);

	indexes.insert(indexes.end(), result.begin(), result.end());
	return indexes;
}

std::vector<AsmIndex::Index> Assembler::IndexInstructions(const std::vector<Instruction>& instructions, const std::vector<uint8_t>& baseBytes)
{
	auto hash = [](const Package& lhs) { return std::hash<Package>::_Do_hash(lhs); };
	auto equal = [](const Package& lhs, const Package& rhs) { return memcmp(&lhs, &rhs, sizeof(lhs)) == 0; };

	auto indexes = std::unordered_map<Package, std::vector<Instruction>, decltype(hash), decltype(equal)>(8, hash, equal);

	for (const Instruction& instruction : instructions)
	{
		indexes[*instruction.m_Package].push_back(instruction);
	}

	std::vector<AsmIndex::Index> result;
	for (const std::pair<Package, std::vector<Instruction>>& index : indexes)
	{
		const Instruction& reference = index.second[0];

		AsmIndex::Index indexed = AsmIndex::Index(*reference.m_Package);
		
		indexed.m_Instruction = reference.m_Byte;
		if (baseBytes.size() != 0)
		{
			indexed.m_HasBase = true;
			indexed.m_Base = baseBytes[0];
		}

		uint8_t mod = ~0;
		uint8_t reg = ~0;
		uint8_t mem = ~0;

		for (int32_t i = reference.m_Chain.size() - 1; i >= 0; i--)
		{
			switch (reference.m_Chain[i].m_Type)
			{
			case ReType::Prefix:
			{
				constexpr uint8_t indexToByte[] = { 0x9B, 0xF3, 0xF2, 0x66, 0x48 };

				bool hasDefault = false;
				for (const Instruction& instruction : index.second)
				{
					if (instruction.m_Chain[i].m_Index == REDIRECT_PREFIX_SIZE - 1)
					{
						hasDefault = true;
						break;
					}
				}

				if (!hasDefault)
				{
					indexed.m_Prefix = indexToByte[reference.m_Chain[i].m_Index];
					indexed.m_HasPrefix = true;
				}
			} break;
			case ReType::Reg:
			{
				bool match[REDIRECT_REG_SIZE] = {};

				for (const Instruction& instruction : index.second)
				{
					if (instruction.m_Chain.size() <= i)
					{
						continue;
					}

					if (instruction.m_Chain[i].m_Type != reference.m_Chain[i].m_Type)
					{
						continue;
					}

					match[reference.m_Chain[i].m_Index] = true;
				}

				bool matched = true;
				for (bool entry : match)
				{
					if (!entry)
					{
						matched = false;
						break;
					}
				}

				if (!matched)
				{
					reg = reference.m_Chain[i].m_Index;
				}
			} break;
			case ReType::Mem:
			{
				bool match[REDIRECT_MEM_SIZE] = {};

				for (const Instruction& instruction : index.second)
				{
					if (instruction.m_Chain.size() <= i)
					{
						continue;
					}

					if (instruction.m_Chain[i].m_Type != reference.m_Chain[i].m_Type)
					{
						continue;
					}

					match[reference.m_Chain[i].m_Index] = true;
				}

				bool matched = true;
				for (bool entry : match)
				{
					if (!entry)
					{
						matched = false;
						break;
					}
				}

				if (!matched)
				{
					mem = reference.m_Chain[i].m_Index;
				}
			} break;
			case ReType::Mod:
			{
				bool match[REDIRECT_MOD_SIZE] = {};

				for (const Instruction& instruction : index.second)
				{
					if (instruction.m_Chain.size() <= i)
					{
						continue;
					}

					if (instruction.m_Chain[i].m_Type != reference.m_Chain[i].m_Type)
					{
						continue;
					}

					match[instruction.m_Chain[i].m_Index] = true;
				}

				bool matched = true;
				for (bool entry : match)
				{
					if (!entry)
					{
						matched = false;
						break;
					}
				}

				if (!matched)
				{
					if (match[0] && match[1] && match[2])
					{
						for (AsmIndex::IndexOperand& operand : indexed.m_Operands)
						{
							if (operand.m_Type == AsmIndex::IndexOpType::none)
							{
								break;
							}

							if (operand.m_Type == AsmIndex::IndexOpType::modrm)
							{
								operand.m_Type = AsmIndex::IndexOpType::mem;
							}
						}
					}
					else
					{
						mod = reference.m_Chain[i].m_Index;
					}
				}
			} break;
			case ReType::X0F383A:
			{
				uint8_t bytes[] = { 0x38, 0x3A };

				if (reference.m_Chain[i].m_Index != X0F383A_None)
				{
					indexed.m_HasX0F383A = true;
					indexed.m_X0F383A = bytes[reference.m_Chain[i].m_Index - 1];
				}
			} break;
			}
		}

		if (mod != static_cast<uint8_t>(~0) &&
			reg != static_cast<uint8_t>(~0) &&
			mem != static_cast<uint8_t>(~0))
		{
			indexed.m_HasModRM = true;
			indexed.m_ModRM = (mod << 6) | ((reg << 3) & ((1 << 3) - 1)) | (mem & ((1 << 3) - 1));
		}
		else
		{
			if (reg != static_cast<uint8_t>(~0))
			{
				indexed.m_ModRM = reg;
				indexed.m_OnlyReg = true;
			}
		}

		result.push_back(indexed);
	}

	return result;
}

void Assembler::FlattenChain(std::vector<Instruction>& instructions, const Package* base, const Package* package)
{
	switch (package->m_Type)
	{
	case PaType::Prefix:
	{
		if (package->m_Prefix.m_Instruction != InsType_invalid)
		{
			instructions.push_back(Instruction(package));
		}
	} break;
	case PaType::Instruction:
	{
		instructions.push_back(Instruction(package));
	} break;
	case PaType::Redirect:
	{
		const Redirect& redirect = package->m_Redirect;

		uint8_t size = 0;
		uint8_t indexSize = 0;
		uint8_t indexOffset = 0;
		switch (redirect.m_Type)
		{
		case ReType::X0F383A:
		{
			size = REDIRECT_X0F383A_SIZE;
			indexSize = Utility::BitSize(REDIRECT_X0F383A_SIZE) - 1;
			indexOffset = (1 << (Utility::BitSize(REDIRECT_X0F383A_SIZE) - 1));
		} break;
		case ReType::Prefix:
		{
			size = REDIRECT_PREFIX_SIZE;
			indexSize = Utility::BitSize(REDIRECT_PREFIX_SIZE) - 1;
			indexOffset = (1 << (Utility::BitSize(REDIRECT_PREFIX_SIZE) - 1));
		} break;
		case ReType::Mod:
		{
			size = REDIRECT_MOD_SIZE;
			indexSize = Utility::BitSize(REDIRECT_MOD_SIZE) - 1;
			indexOffset = (1 << (Utility::BitSize(REDIRECT_MOD_SIZE) - 1));
		} break;
		case ReType::Reg:
		{
			size = REDIRECT_REG_SIZE;
			indexSize = Utility::BitSize(REDIRECT_REG_SIZE) - 1;
			indexOffset = (1 << (Utility::BitSize(REDIRECT_REG_SIZE) - 1));
		} break;
		case ReType::Mem:
		{
			size = REDIRECT_MEM_SIZE;
			indexSize = Utility::BitSize(REDIRECT_MEM_SIZE) - 1;
			indexOffset = (1 << (Utility::BitSize(REDIRECT_MEM_SIZE) - 1));
		} break;
		}

		for (uint8_t i = 0; i < size; i++)
		{
			if ((redirect.m_Mask & (1 << i)) == 0)
			{
				continue;
			}

			uint8_t offset = (redirect.m_Mask >> (indexOffset + (indexSize * i))) & ((1 << indexSize) - 1);

			uint32_t index = instructions.size();

			FlattenChain(instructions, base, base + offset + redirect.m_BaseIndex);
			
			for (uint32_t ii = index; ii < instructions.size(); ii++)
			{
				instructions[ii].m_Chain.push_back(Level(redirect.m_Type, i));
			}
		}

	} break;
	}
}