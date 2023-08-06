#pragma once
#include "ArchitectureIL.hpp"
#include "AsmIndex.hpp"
#include <vector>

class Assembler
{
public:

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

		std::vector<Level> m_Chain; // redirect chain
		const Package* m_Package = nullptr;
	};

public:
	Assembler();

private:
	std::vector<AsmIndex::Index> FlattenInstructions(const Package* base, const Package* package, std::vector<uint8_t>& baseBytes);

private:
	std::vector<AsmIndex::Index> IndexInstructions(const std::vector<Instruction>& instructions, const std::vector<uint8_t>& baseBytes);

	void FlattenChain(std::vector<Instruction>& instructions, const Package* base, const Package* package);

private:
	static std::shared_ptr<AsmIndex> s_ReverseTable[InsType_ARRAY_MAX];
};