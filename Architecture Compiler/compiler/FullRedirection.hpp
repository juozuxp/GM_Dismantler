#pragma once
#include <vector>
#include <stdint.h>
#include "ByteEntry.hpp"

class FullRedirection : public ByteEntry
{
public:
#pragma pack(push, 1)
	struct Package
	{
		uint16_t m_BaseIndex;
	};
#pragma pack(pop)

public:
	PackageType GetPackageType() const override;
	class BytePackage GetPackage(uint32_t freeSpace) const override;

public:
	std::shared_ptr<ByteEntry>& Get(uint8_t index);

private:
	Package GetBasePackage(uint32_t freeSpace) const;

private:
	std::shared_ptr<ByteEntry> m_Entries[256];
};