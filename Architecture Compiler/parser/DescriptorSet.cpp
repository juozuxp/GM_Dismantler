#include "DescriptorSet.hpp"

DescriptorSet::DescriptorSet(const TokenSet& tokens)
{
	for (const Token& token : tokens)
	{
		m_Descriptors.push_back(Descriptor(token));
	}
}

std::vector<Descriptor>::const_iterator DescriptorSet::end() const
{
	return m_Descriptors.end();
}

std::vector<Descriptor>::const_iterator DescriptorSet::begin() const
{
	return m_Descriptors.begin();
}