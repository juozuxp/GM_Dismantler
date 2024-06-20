#include "Prefix.hpp"
#include "BytePackage.hpp"
#include <assert.h>

Prefix::Prefix(uint32_t prefix) :
	m_Prefix(prefix)
{
	if (prefix & Type_RexW)
	{
		m_RedirectPrefix = Redirection::Prefix::RexW;
		return;
	}

	if (prefix & Type_Wait)
	{
		m_RedirectPrefix = Redirection::Prefix::Wait;
		return;
	}

	if (prefix & Type_Repe)
	{
		m_RedirectPrefix = Redirection::Prefix::Repe;
		return;
	}

	if (prefix & Type_Repne)
	{
		m_RedirectPrefix = Redirection::Prefix::Repne;
		return;
	}

	if (prefix & Type_x66)
	{
		m_RedirectPrefix = Redirection::Prefix::x66;
		return;
	}
}

BytePackage Prefix::GetPackage(uint32_t freeSpace) const
{
	return BytePackage(GetBasePackage());
}

Prefix::Package Prefix::GetBasePackage() const
{
	Package package;

	package.m_Prefix = m_Prefix;
	package.m_Instruction = m_Instruction;

	return package;
}

void Prefix::SetStandAlone(uint16_t instructionType)
{
	m_Instruction = instructionType;
}

bool Prefix::IsStandAlone()
{
	return m_Instruction != 0;
}

Prefix::PackageType Prefix::GetPackageType() const
{
	return PackageType::Prefix;
}

Redirection::Prefix Prefix::GetRedirectPrefix() const
{
	assert(m_RedirectPrefix != Redirection::Prefix::Default);
	return m_RedirectPrefix;
}