#include "Packager.hpp"

Package Packager::AssemblePackage() const
{
	TokenSet tokens = TokenSet();

	for (const std::wstring& file : m_Paths)
	{
		std::ifstream stream = std::ifstream(file);

		tokens << TokenSet(stream);
	}

	DescriptorSet descriptors = DescriptorSet(tokens);
	InstructionSet instructions = InstructionSet(descriptors);

	return Package(instructions);
}