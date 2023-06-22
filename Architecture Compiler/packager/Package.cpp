#include "Package.hpp"

#define NEW_LINE_BREAK 0x10

Package::Package(const InstructionSet& instructions)
{
	std::vector<BytePackage::Raw> raw = instructions.GetPackage().GetRaw();

	m_ByteCode.resize(raw.size() * sizeof(BytePackage::Raw));
	memcpy(m_ByteCode.data(), raw.data(), raw.size() * sizeof(BytePackage::Raw));

	m_Types = instructions.GetTypes();
}

void Package::OutputToHeader(const std::string_view& filePath) const
{
	std::ofstream stream = std::ofstream(filePath.data());

	stream << "#include <stdint.h>\n\n" + GetEnumTypes() + "\n\n" + GetArrayTypeStrings() + "\n\n" + GetArrayByteCode();
}

std::string Package::GetEnumTypes() const
{
	std::string enumerator = "enum Type\n{\n";

	for (const std::string& type : m_Types)
	{
		enumerator += "\tType_";
		enumerator += type;
		enumerator += ",\n";
	}

	return enumerator + "};";
}

std::string Package::GetArrayByteCode() const
{
	std::string array = "constexpr static uint8_t Package[] = \n{\n\t";

	uint8_t newLine = 0;
	for (uint8_t byte : m_ByteCode)
	{
		if (NEW_LINE_BREAK <= newLine)
		{
			newLine = 1;

			array.erase(array.size() - 1);
			array += "\n\t";
		}
		else
		{
			newLine++;
		}

		char first = (byte >> 4);
		char second = (byte & ((1 << 4) - 1));

		first += first < 10 ? '0' : ('A' - 10);
		second += second < 10 ? '0' : ('A' - 10);

		array += "0x";
		array += first;
		array += second;

		array += ", ";
	}

	array.erase(array.size() - 2);
	return array + "\n};";
}

std::string Package::GetArrayTypeStrings() const
{
	std::string array = "constexpr static const char* TypeToName[] = \n{\n\t";

	uint8_t newLine = 0;
	for (const std::string& type : m_Types)
	{
		if (NEW_LINE_BREAK <= newLine)
		{
			newLine = 1;

			array.erase(array.size() - 1);
			array += "\n\t";
		}
		else
		{
			newLine++;
		}

		array += "\"";
		array += type;
		array += "\", ";
	}

	array.erase(array.size() - 2);
	return array + "\n};";
}