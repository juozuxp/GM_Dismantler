#include "Prefix.hpp"
#include "BytePackage.hpp"

Prefix::Prefix(uint16_t prefix) :
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

	if (prefix & Type_x0F38)
	{
		m_RedirectPrefix = Redirection::Prefix::x0F38;
		return;
	}

	if (prefix & Type_x0F3A)
	{
		m_RedirectPrefix = Redirection::Prefix::x0F3A;
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

	package.m_Value = m_Prefix;

	return package;
}

Prefix::PackageType Prefix::GetPackageType() const
{
	return PackageType::Prefix;
}

Redirection::Prefix Prefix::GetRedirectPrefix() const
{
	return m_RedirectPrefix;
}