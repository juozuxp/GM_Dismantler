#pragma once
#include "ArchitectureIL.hpp"
#include <vector>

class Visualizer
{
public:
	void PrintToConsole(const std::vector<ILInstruction>& instructions);
};