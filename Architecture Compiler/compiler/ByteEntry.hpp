#pragma once
#include <memory>

class ByteEntry : std::enable_shared_from_this<ByteEntry>
{
public:
	enum class PackageType : uint8_t
	{
		Invalid,
		Prefix,
		Redirection,
		Instruction,
		FullRedirection
	};

public:
	virtual PackageType GetPackageType() const = 0;
	virtual class BytePackage GetPackage(uint32_t freeSpace) const = 0;
};