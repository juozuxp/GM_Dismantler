#include "lexer/TokenSet.hpp"
#include "parser/DescriptorSet.hpp"
#include "compiler/InstructionSet.hpp"
#include "packager/Packager.hpp"
#include <filesystem>

int main()
{
	Packager package = Packager();

	for (std::filesystem::directory_entry file : std::filesystem::directory_iterator("architecture"))
	{
		if (file.path().extension() == L".asm64")
		{
			package.AddPackages(file.path().c_str());
		}
	}

	package.AssemblePackage().OutputToHeader("../Architect/include/x86_x64.hpp");
}