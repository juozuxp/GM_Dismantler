#include <Disassembler.hpp>

int main()
{
	uint8_t code[] = { 0x00, 0xFF };

	Disassembler instance;

	std::vector<ILInstruction> instructions = instance.Disassemble(code, sizeof(code));
}