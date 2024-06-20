#pragma once
#include <string>
#include <vector>

class Token
{
public:
	Token() = default;

	Token(const std::string_view& line);

public:
	constexpr const std::string& GetName() const
	{
		return m_Instruction;
	}

	constexpr const std::vector<std::string>& GetBytes() const
	{
		return m_Bytes;
	}

	constexpr const std::vector<std::vector<std::string>>& GetOperands() const
	{
		return m_Operands;
	}

private:
	std::string m_Instruction;
	std::vector<std::string> m_Bytes;
	std::vector<std::vector<std::string>> m_Operands;
};