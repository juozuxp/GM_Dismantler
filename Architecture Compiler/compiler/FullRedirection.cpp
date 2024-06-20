#include "FullRedirection.hpp"
#include "BytePackage.hpp"
#include "tools/ArraySize.h"

FullRedirection::PackageType FullRedirection::GetPackageType() const
{
	return PackageType::FullRedirection;
}

BytePackage FullRedirection::GetPackage(uint32_t freeSpace) const
{
	BytePackage package = BytePackage(GetBasePackage(freeSpace));

	freeSpace += ARRAY_SIZE(m_Entries);
	for (uint32_t i = 0; i < ARRAY_SIZE(m_Entries); i++)
	{
		if (!m_Entries[i])
		{
			package.AddChild(BytePackage());
			continue;
		}

		BytePackage entryPackage = m_Entries[i]->GetPackage(freeSpace);

		freeSpace += entryPackage.GetCount() - 1;

		package.AddChild(entryPackage);
	}

	return package;
}

FullRedirection::Package FullRedirection::GetBasePackage(uint32_t freeSpace) const
{
	Package package;

	package.m_BaseIndex = freeSpace;

	return package;
}

std::shared_ptr<ByteEntry>& FullRedirection::Get(uint8_t index)
{
	return m_Entries[index];
}