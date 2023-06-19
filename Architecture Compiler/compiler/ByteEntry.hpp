#pragma once
#include <memory>

class ByteEntry : std::enable_shared_from_this<ByteEntry>
{
public:
	enum class PackageType
	{
		Invalid,
		Prefix,
		Instruction,
		Redirection,
		FullRedirection
	};

public:
	virtual PackageType GetPackageType() const = 0;
	virtual class BytePackage GetPackage(uint32_t freeSpace) const = 0;
};