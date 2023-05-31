#pragma once
#include "lexer/TokenSet.hpp"
#include "Descriptor.hpp"

class DescriptorSet
{
public:
	DescriptorSet(const TokenSet& tokens);

private:
	std::vector<Descriptor> m_Descriptors;
};

