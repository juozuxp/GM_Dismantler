#include <Disassembler.hpp>
#include <Visualizer.hpp>

int main()
{
	uint8_t code[] = { 0x0F, 0x01, 0xC1 };

	Disassembler instance;
	Visualizer visualizer;

	std::vector<ILInstruction> instructions = instance.Disassemble(code, 1);

	visualizer.PrintToConsole(instructions);
}