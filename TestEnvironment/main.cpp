#include <Disassembler.hpp>
#include <Visualizer.hpp>
#include <Windows.h>
#include <string_view>


const IMAGE_SECTION_HEADER* GetSection(HMODULE module, const std::string_view& name)
{
	const IMAGE_DOS_HEADER* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
	const IMAGE_NT_HEADERS* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(reinterpret_cast<const uint8_t*>(dos) + dos->e_lfanew);

	const IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);

	for (uint32_t i = 0; i < nt->FileHeader.NumberOfSections; i++, section++)
	{
		if (name == reinterpret_cast<const char*>(section->Name))
		{
			return section;
		}
	}

	return nullptr;
}

int main()
{

	uint8_t code[] = { 0x0F, 0x01, 0xC1 };

	Disassembler instance;
	Visualizer visualizer;


	HMODULE module = LoadLibraryA("ntdll.dll");
	const IMAGE_SECTION_HEADER* section = GetSection(module, ".text");

	std::vector<ILInstruction> instructions = instance.Disassemble(reinterpret_cast<const uint8_t*>(module) + section->VirtualAddress, section->Misc.VirtualSize);


	printf("%p\n", module);

	uint64_t index = 0;
	uint64_t cursor = reinterpret_cast<uint64_t>(module) + section->VirtualAddress;
	for (const ILInstruction& instruction : instructions)
	{
		if (instruction.m_Type == InsType_invalid)
		{
			printf("Invalid at: %p\n", cursor);
		}

		index++;
		cursor += instruction.m_Size;
	}

	// visualizer.PrintToConsole(instructions);

	system("pause");
}