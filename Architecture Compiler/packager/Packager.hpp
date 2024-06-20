#pragma once
#include "compiler/InstructionSet.hpp"
#include "Package.hpp"

class Packager
{
public:
	Packager() = default;

public:
	Package AssemblePackage() const;

public:
	template<typename T = std::wstring, typename... Types>
	void AddPackages(const T& filePath, Types... paths)
	{
		m_Paths.push_back(filePath);
		AddPackages(paths...);
	}

private:
	void AddPackages()
	{
	}

private:
	std::vector<std::wstring> m_Paths;
};