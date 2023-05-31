#include "DescriptorSet.hpp"

DescriptorSet::DescriptorSet(const TokenSet& tokens)
{
	for (const Token& token : tokens)
	{
		m_Descriptors.push_back(Descriptor(token));
	}
}