#pragma once
#include "ArchitectureIL.hpp"
#include "AsmIndex.hpp"
#include <vector>

class Assembler
{
private:
	struct Level
	{
		Level() = default;
		Level(ReType type, uint32_t index);

		ReType m_Type = ReType::None;
		uint32_t m_Index = 0;
	};

	struct Instruction
	{
		Instruction() = default;
		Instruction(const Package* package);

		uint8_t m_Byte = 0;
		const Package* m_Package = nullptr;
		std::vector<Level> m_Chain; // redirect chain
	};

public:
	Assembler();

public:
	std::vector<uint8_t> Assemble(const std::vector<ILInstruction>& instructions) const;

private:
	std::vector<AsmIndex::Index> FlattenInstructions(const Package* base, const Package* package, std::vector<uint8_t>& baseBytes);

private:
	void FlattenChain(std::vector<Instruction>& instructions, const Package* base, const Package* package);

	std::vector<AsmIndex::Index> IndexInstructions(const std::vector<Instruction>& instructions, const std::vector<uint8_t>& baseBytes);

private:
	static bool s_ReverseTableValid;
	static std::shared_ptr<AsmIndex> s_ReverseTable[InsType_ARRAY_MAX];
};