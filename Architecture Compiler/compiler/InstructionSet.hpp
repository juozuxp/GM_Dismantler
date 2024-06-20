#pragma once
#include <map>
#include "parser/DescriptorSet.hpp"
#include "ByteEntry.hpp"
#include "BytePackage.hpp"

class InstructionSet
{
public:
	InstructionSet() = default;
	InstructionSet(const DescriptorSet& descriptors);

public:
	BytePackage GetPackage() const;
	std::vector<std::string> GetTypes() const;

private:
	void Insert(const Descriptor& descriptor);
	
private:
	void CreateCoreBytes();

private:
	std::shared_ptr<ByteEntry> m_Bytes[256];
	std::map<std::string, uint32_t> m_Types;
};