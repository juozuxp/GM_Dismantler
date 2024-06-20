#pragma once
#include <memory>
#include <string_view>
#include "compiler/InstructionSet.hpp"

class Package
{
public:
	Package() = default;
	Package(const InstructionSet& instructions);

public:
	void OutputToHeader(const std::string_view& filePath) const;

private:
	std::string GetEnumTypes() const;
	std::string GetArrayByteCode() const;
	std::string GetCoreStructures() const;
	std::string GetArrayTypeStrings() const;

private:
	std::vector<uint8_t> m_ByteCode;
	std::vector<std::string> m_Types;
};