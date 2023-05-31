#include "TokenSet.hpp"
#include <sstream>

TokenSet::TokenSet(std::ifstream& file)
{
	std::string line;

	while (std::getline(file, line))
	{
		if (line.empty())
		{
			continue;
		}

		ProcessLine(line);
	}
}

TokenSet::TokenSet(std::string text)
{
	std::string line;
	std::stringstream textStream(text);

	while (std::getline(textStream, line))
	{
		if (line.empty())
		{
			continue;
		}

		ProcessLine(line);
	}
}

TokenSet& TokenSet::operator<<(const TokenSet& lhs)
{
	return operator+=(lhs);
}

TokenSet& TokenSet::operator+=(const TokenSet& lhs)
{
	m_Tokens.insert(m_Tokens.end(), lhs.m_Tokens.begin(), lhs.m_Tokens.end());
	return *this;
}

void TokenSet::ProcessLine(const std::string& line)
{
	m_Tokens.push_back(Token(line));
}