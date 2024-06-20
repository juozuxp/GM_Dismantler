#pragma once
#include "lexer/TokenSet.hpp"
#include "Descriptor.hpp"

class DescriptorSet
{
public:
	DescriptorSet(const TokenSet& tokens);

public:
	std::vector<Descriptor>::const_iterator end() const;
	std::vector<Descriptor>::const_iterator begin() const;

private:
	std::vector<Descriptor> m_Descriptors;
};