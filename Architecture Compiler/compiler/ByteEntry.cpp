#include "ByteEntry.hpp"

ByteEntry::ByteEntry(const Prefix& prefix) :
	m_Type(PackageType::Prefix), m_Prefix(prefix)
{
}

ByteEntry::ByteEntry(const Instruction& instruction) :
	m_Type(PackageType::Instruction), m_Instruction(instruction)
{
}

ByteEntry::ByteEntry(const Redirection& redirection) :
	m_Type(PackageType::Redirection), m_Redirection(redirection)
{
}

ByteEntry::Package ByteEntry::GetPackage() const
{

}