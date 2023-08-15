#include "AsmIndex.hpp"
#include "Utility.hpp"
#include "AsmInstruction.hpp"
#include "AsmRedirectType.hpp"
#include "AsmRedirectSize.hpp"
#include "AsmRedirectConstant.hpp"
#include "AsmRedirectRegister.hpp"
#include <map>

std::shared_ptr<AsmIndex> AsmIndex::MapTree(const std::vector<Index>& indexes)
{
	if (indexes.size() == 0)
	{
		return std::shared_ptr<AsmIndex>();
	}

	if (indexes.size() == 1)
	{
		return std::make_shared<AsmInstruction>(indexes[0]);
	}

	return MapOperandType(DeSynonymise(indexes), 0);
}

std::vector<AsmIndex::Index> AsmIndex::DeSynonymise(const std::vector<Index>& indexes)
{
	std::vector<Index> result = indexes;

	for (Index& index : result)
	{
		for (IndexOperand& operand : index.m_Operands)
		{
			if (operand.m_Type == IndexOpType::modrm)
			{
				operand.m_Type = IndexOpType::reg;
				operand.m_Size[static_cast<uint8_t>(IndexOpSize::mem)].m_Value = 0;
			}
		}
	}

	for (uint32_t i = 0; i < result.size(); i++)
	{
		Index& first = result[i];

		bool match = false;
		for (uint32_t j = i + 1; j < result.size(); j++)
		{
			Index& second = result[j];

			if (memcmp(first.m_Operands, second.m_Operands, sizeof(first.m_Operands)) == 0)
			{
				match = true;
				break;
			}
		}

		if (match)
		{
			for (uint8_t j = 0; j < ARRAY_SIZE(Index::m_Operands); j++)
			{
				if (indexes[i].m_Operands[j].m_Type == IndexOpType::modrm)
				{
					first.m_Operands[j].m_Type = IndexOpType::mem;
					first.m_Operands[j].m_Size[static_cast<uint8_t>(IndexOpSize::mem)].m_Value = indexes[i].m_Operands[j].m_Size[static_cast<uint8_t>(IndexOpSize::mem)].m_Value;
				}
			}
		}
		else
		{
			first = indexes[i];
		}
	}

	return result;
}

std::shared_ptr<AsmIndex> AsmIndex::MapOperandType(const std::vector<Index>& indexes, uint8_t first)
{
	std::map<IndexOpType, std::vector<Index>> typeMap;
	
	const Index& reference = indexes[0];
	for (uint8_t i = first; i < ARRAY_SIZE(Index::m_Operands); i++)
	{
		bool differ = false;
		for (const Index& index : indexes)
		{
			if (index.m_Operands[i].m_Type == IndexOpType::modrm)
			{
				Index mem = index;
				Index reg = index;

				mem.m_Operands[i].m_Type = IndexOpType::mem;
				reg.m_Operands[i].m_Size[static_cast<uint8_t>(IndexOpSize::mem)] = reg.m_Operands[i].m_Size[static_cast<uint8_t>(IndexOpSize::reg)];

				typeMap[IndexOpType::mem].push_back(mem);
				typeMap[IndexOpType::reg].push_back(reg);
			}
			else
			{
				typeMap[index.m_Operands[i].m_Type].push_back(index);
			}

			if (index.m_Operands[i].m_Type != reference.m_Operands[i].m_Type)
			{
				differ = true;
			}
		}

		if (differ)
		{
			std::shared_ptr<AsmRedirectType> redirect = std::make_shared<AsmRedirectType>(i);

			for (std::pair<IndexOpType, std::vector<Index>> type : typeMap)
			{
				if (type.second.size() == 1)
				{
					redirect->Set(type.first, std::make_shared<AsmInstruction>(type.second[0]));
					continue;
				}

				redirect->Set(type.first, MapOperandType(type.second, i + 1));
			}

			return redirect;
		}

		typeMap.clear();
	}

	return MapOperandSize(indexes, 0);
}

std::shared_ptr<AsmIndex> AsmIndex::MapOperandSize(const std::vector<Index>& indexes, uint8_t first)
{
	std::map<OpSize, std::vector<Index>> sizeMap;

	auto insertToMap = [&](OpSize size, const Index& index) 
	{
		std::vector<Index>& array = sizeMap[size];

		if (array.size() != 0)
		{
			if (array[array.size() - 1] == index)
			{
				return;
			}
		}

		sizeMap[size].push_back(index);
		return;
	};

	const Index& reference = indexes[0];
	for (uint8_t i = first; i < ARRAY_SIZE(Index::m_Operands); i++)
	{
		if (reference.m_Operands[i].m_Type == IndexOpType::none)
		{
			break;
		}

		bool differ = false;
		for (const Index& index : indexes)
		{
			for (uint8_t j = 0; j < ARRAY_SIZE(IndexOperand::m_Size); j++)
			{
				if (reference.m_Operands[i].m_Type == IndexOpType::reg)
				{
					if (j == static_cast<uint8_t>(IndexOpSize::mem))
					{
						continue;
					}
				}
				else if (reference.m_Operands[i].m_Type != IndexOpType::modrm)
				{
					if (j == static_cast<uint8_t>(IndexOpSize::reg))
					{
						continue;
					}
				}
				
				const OperandSize& size = index.m_Operands[i].m_Size[j];
				if (size.m_Value != reference.m_Operands[i].m_Size[j].m_Value)
				{
					differ = true;
				}

				switch (size.m_Size)
				{
				case OpSize::base_16:
				{
					if (size.m_Override1)
					{
						insertToMap(OpSize::base_32, index);
					}

					if (size.m_Override2)
					{
						insertToMap(OpSize::base_64, index);
					}
				} break;
				case OpSize::base_32:
				{
					if (size.m_Override1)
					{
						insertToMap(OpSize::base_16, index);
					}

					if (size.m_Override2)
					{
						insertToMap(OpSize::base_64, index);
					}
				} break;
				case OpSize::base_64:
				{
					if (size.m_Override1)
					{
						insertToMap(OpSize::base_16, index);
					}
				} break;
				case OpSize::base_128:
				{
					if (size.m_Override1)
					{
						insertToMap(OpSize::base_256, index);
					}

					if (size.m_Override2)
					{
						insertToMap(OpSize::base_512, index);
					}
				} break;
				}

				insertToMap(size.m_Size, index);
			}
		}

		if (differ)
		{
			std::shared_ptr<AsmRedirectSize> redirect = std::make_shared<AsmRedirectSize>(i);

			for (std::pair<OpSize, std::vector<Index>> size : sizeMap)
			{
				if (size.second.size() == 1)
				{
					redirect->Set(size.first, std::make_shared<AsmInstruction>(size.second[0]));
					continue;
				}

				redirect->Set(size.first, MapOperandSize(size.second, i + 1));
			}

			return redirect;
		}

		sizeMap.clear();
	}

	return MapOperandConstant(indexes, 0);
}

std::shared_ptr<AsmIndex> AsmIndex::MapOperandConstant(const std::vector<Index>& indexes, uint8_t first)
{
	std::map<uint8_t, std::vector<Index>> indexMap;

	const Index& reference = indexes[0];
	for (uint8_t i = first; i < ARRAY_SIZE(Index::m_Operands); i++)
	{
		if (reference.m_Operands[i].m_Type == IndexOpType::none)
		{
			break;
		}

		bool differ = false;
		for (const Index& index : indexes)
		{
			if (index.m_Operands[i].m_Constant != reference.m_Operands[i].m_Constant)
			{
				differ = true;
			}
			else if (index.m_Operands[i].m_Value != reference.m_Operands[i].m_Value)
			{
				differ = true;
			}

			if (index.m_Operands[i].m_Constant)
			{
				indexMap[index.m_Operands[i].m_Value].push_back(index);

				if (index.m_Operands[i].m_Type == IndexOpType::reg &&
					index.m_Operands[i].m_Rex)
				{
					indexMap[index.m_Operands[i].m_Value + 8].push_back(index);
				}
			}
			else
			{
				indexMap[~0].push_back(index);
			}
		}

		if (differ)
		{
			std::shared_ptr<AsmRedirectConstant> redirect = std::make_shared<AsmRedirectConstant>(i);

			for (std::pair<uint8_t, std::vector<Index>> index : indexMap)
			{
				if (index.second.size() == 1)
				{
					redirect->Set(index.first, std::make_shared<AsmInstruction>(index.second[0]));
					continue;
				}

				redirect->Set(index.first, MapOperandConstant(index.second, i + 1));
			}

			return redirect;
		}

		indexMap.clear();
	}

	return MapOperandRegister(indexes, 0);
}

std::shared_ptr<AsmIndex> AsmIndex::MapOperandRegister(const std::vector<Index>& indexes, uint8_t first)
{
	std::map<Register, std::vector<Index>> registerMap;

	const Index& reference = indexes[0];
	for (uint8_t i = first; i < ARRAY_SIZE(Index::m_Operands); i++)
	{
		if (reference.m_Operands[i].m_Type == IndexOpType::none)
		{
			break;
		}

		bool differ = false;
		for (const Index& index : indexes)
		{
			if (index.m_Operands[i].m_Register != reference.m_Operands[i].m_Register)
			{
				differ = true;
			}

			registerMap[index.m_Operands[i].m_Register].push_back(index);
		}

		if (differ)
		{
			std::shared_ptr<AsmRedirectRegister> redirect = std::make_shared<AsmRedirectRegister>(i);

			for (std::pair<Register, std::vector<Index>> reg : registerMap)
			{
				if (reg.second.size() == 1)
				{
					redirect->Set(reg.first, std::make_shared<AsmInstruction>(reg.second[0]));
					continue;
				}

				redirect->Set(reg.first, MapOperandRegister(reg.second, i + 1));
			}

			return redirect;
		}

		registerMap.clear();
	}

	return std::shared_ptr<AsmIndex>();
}

AsmIndex::IndexOperand::IndexOperand(const Operand& operand)
{
	constexpr IndexOpType types[] = { IndexOpType::none, IndexOpType::reg, IndexOpType::modrm, IndexOpType::imm, IndexOpType::rel, IndexOpType::moffs };

	m_Type = types[static_cast<uint8_t>(operand.m_Type)];
	m_Register = operand.m_Register;

	m_Rex = operand.m_Rex;
	m_Constant = operand.m_Constant;
	m_Value = operand.m_Value;

	m_Size[static_cast<uint8_t>(IndexOpSize::reg)] = operand.m_Reg;
	m_Size[static_cast<uint8_t>(IndexOpSize::mem)] = operand.m_Mem;
}

AsmIndex::Index::Index(const Package& package)
{
	switch (package.m_Type)
	{
	case PaType::Instruction:
	{
		m_Type = package.m_Instruction.m_Type;

		uint8_t count = 0;
		for (const Operand& operand : package.m_Instruction.m_Operands)
		{
			if (operand.m_Type == OpType::none)
			{
				break;
			}

			m_Operands[count++] = IndexOperand(operand);
		}
	} break;
	case PaType::Prefix:
	{
		_ASSERT(package.m_Prefix.m_Instruction != InsType_invalid);
		m_Type = package.m_Prefix.m_Instruction;
	} break;
	}
}


bool AsmIndex::Index::operator==(const Index& lhs)
{
	return m_Comparer == lhs.m_Comparer;
}

bool AsmIndex::Index::operator!=(const Index& lhs)
{
	return !operator==(lhs);
}