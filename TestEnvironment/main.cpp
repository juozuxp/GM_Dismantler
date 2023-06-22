#include <Disassembler.hpp>
#include <Visualizer.hpp>

int main()
{
	uint8_t code[] = { 0xF3, 0x0F, 0x01, 0xEE };

	Disassembler instance;
	Visualizer visualizer;

	std::vector<ILInstruction> instructions = instance.Disassemble(code, sizeof(code));

	visualizer.PrintToConsole(instructions);
}