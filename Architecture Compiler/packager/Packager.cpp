#include "Packager.hpp"

Package Packager::AssemblePackage() const
{
	TokenSet tokens = TokenSet();

	for (const std::wstring& file : m_Paths)
	{
		tokens << TokenSet(std::ifstream(file));
	}

	DescriptorSet descriptors = DescriptorSet(tokens);
	InstructionSet instructions = InstructionSet(descriptors);

	return Package(instructions);
}