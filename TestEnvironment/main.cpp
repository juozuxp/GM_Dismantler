#include <Disassembler.hpp>
#include <Visualizer.hpp>

int main()
{
	uint8_t code[] = { 0xD9, 0xFE };

	Disassembler instance;
	Visualizer visualizer;

	std::vector<ILInstruction> instructions = instance.Disassemble(code, 1);

	visualizer.PrintToConsole(instructions);
}