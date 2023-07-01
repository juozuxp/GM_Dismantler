#include <Disassembler.hpp>
#include <Visualizer.hpp>

int main()
{
	uint8_t code[] = { 0x48, 0x83, 0xEC, 0x30 };

	Disassembler instance;
	Visualizer visualizer;

	std::vector<ILInstruction> instructions = instance.Disassemble(code, 1);

	visualizer.PrintToConsole(instructions);
}