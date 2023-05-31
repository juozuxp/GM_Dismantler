#pragma once
#include <vector>
#include <string_view>
#include <fstream>

#include "Token.hpp"

class TokenSet
{
public:
	TokenSet() = default;

	TokenSet(std::string text);
	TokenSet(std::ifstream& file);

public:
	TokenSet& operator<<(const TokenSet& lhs);
	TokenSet& operator+=(const TokenSet& lhs);

public:
	std::vector<Token>::const_iterator begin() const
	{
		return m_Tokens.begin();
	}

	std::vector<Token>::const_iterator end() const
	{
		return m_Tokens.end();
	}

private:
	void ProcessLine(const std::string& line);

private:
	std::vector<Token> m_Tokens;
};