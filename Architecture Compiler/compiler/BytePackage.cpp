#include "BytePackage.hpp"

BytePackage::Raw::Raw(const Prefix::Package& package) :
	m_Type(ByteEntry::PackageType::Prefix), m_Prefix(package)
{
}

BytePackage::Raw::Raw(const Instruction::Package& package) :
	m_Type(ByteEntry::PackageType::Instruction), m_Instruction(package)
{
}

BytePackage::Raw::Raw(const Redirection::Package& package) :
	m_Type(ByteEntry::PackageType::Redirection), m_Redirection(package)
{
}

BytePackage::Raw::Raw(const FullRedirection::Package& package) :
	m_Type(ByteEntry::PackageType::FullRedirection), m_FullRedirection(package)
{
}

BytePackage::BytePackage(const Prefix::Package& package) :
	m_Main(package)
{
}

BytePackage::BytePackage(const Instruction::Package& package) :
	m_Main(package)
{
}

BytePackage::BytePackage(const Redirection::Package& package) :
	m_Main(package)
{
}

BytePackage::BytePackage(const FullRedirection::Package& package) :
	m_Main(package)
{
}

void BytePackage::AddChild(const BytePackage& package)
{
	m_Children.push_back(package);
}

uint32_t BytePackage::GetCount() const
{
	uint32_t count = 1;
	for (const BytePackage& package : m_Children)
	{
		count += package.GetCount();
	}

	return count;
}

std::vector<BytePackage::Raw> BytePackage::GetRaw() const
{
	std::vector<BytePackage::Raw> sidePacakges;
	std::vector<BytePackage::Raw> mainPackages;

	mainPackages.reserve(m_Children.size());
	for (BytePackage package : m_Children)
	{
		std::vector<BytePackage::Raw> raw = package.GetRawChain();

		mainPackages.push_back(raw[0]);
		sidePacakges.insert(sidePacakges.end(), raw.begin() + 1, raw.end());
	}

	mainPackages.insert(mainPackages.end(), sidePacakges.begin(), sidePacakges.end());
	return mainPackages;
}

std::vector<BytePackage::Raw> BytePackage::GetRawChain() const
{
	std::vector<BytePackage::Raw> sidePacakges;
	std::vector<BytePackage::Raw> mainPackages;

	mainPackages.reserve(m_Children.size() + 1);

	mainPackages.push_back(m_Main);
	for (BytePackage package : m_Children)
	{
		std::vector<BytePackage::Raw> raw = package.GetRawChain();

		mainPackages.push_back(raw[0]);
		sidePacakges.insert(sidePacakges.end(), raw.begin() + 1, raw.end());
	}

	mainPackages.insert(mainPackages.end(), sidePacakges.begin(), sidePacakges.end());
	return mainPackages;
}