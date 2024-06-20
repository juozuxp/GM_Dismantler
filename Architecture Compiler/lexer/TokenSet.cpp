#include "TokenSet.hpp"
#include <sstream>

TokenSet::TokenSet(std::ifstream&& file)
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

TokenSet::TokenSet(const std::string_view& text)
{
	std::stringstream textStream(text.data());

	std::string line;
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
	Token token = Token(line);
	if (token.GetBytes().empty())
	{
		return;
	}

	m_Tokens.push_back(token);
}