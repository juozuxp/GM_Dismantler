#pragma once
#include "Instruction.hpp"
#include "ByteEntry.hpp"

#include <stdint.h>
#include <memory>
#include <vector>

class Redirection : public ByteEntry
{
public:
	enum class Type : uint8_t
	{
		None,
		X0F383A,
		Prefix,
		Mod,
		Reg,
		Mem
	};

	enum class X0F383A
	{
		Default,
		x0F38,
		x0F3A,
		ArrayMAX
	};

	enum class Prefix : uint8_t
	{
		Wait,
		Repe,
		Repne,
		x66,
		RexW,
		Default,
		ArrayMAX
	};

	struct Entry
	{
		Entry() = default;
		Entry(Prefix prefix);
		Entry(X0F383A x0F383A);
		Entry(Type type, uint8_t index);

		Type m_Type;

		union
		{
			Prefix m_Prefix;
			uint8_t m_Index;
			X0F383A m_X0F383A;
		};
	};

#pragma pack(push, 1)
	struct Package
	{
		Type m_Type;
		uint32_t m_BaseIndex;
		union
		{
			union
			{
				uint32_t m_Value;
				struct
				{
					uint32_t m_Valid00 : 1;
					uint32_t m_Valid01 : 1;
					uint32_t m_Valid10 : 1;
					uint32_t m_Valid11 : 1;

					uint32_t m_Index00 : 2;
					uint32_t m_Index01 : 2;
					uint32_t m_Index10 : 2;
					uint32_t m_Index11 : 2;
				};
			} m_Mod;
			union
			{
				uint32_t m_Value;
				struct
				{
					uint32_t m_Valid000 : 1;
					uint32_t m_Valid001 : 1;
					uint32_t m_Valid010 : 1;
					uint32_t m_Valid011 : 1;
					uint32_t m_Valid100 : 1;
					uint32_t m_Valid101 : 1;
					uint32_t m_Valid110 : 1;
					uint32_t m_Valid111 : 1;

					uint32_t m_Index000 : 3;
					uint32_t m_Index001 : 3;
					uint32_t m_Index010 : 3;
					uint32_t m_Index011 : 3;
					uint32_t m_Index100 : 3;
					uint32_t m_Index101 : 3;
					uint32_t m_Index110 : 3;
					uint32_t m_Index111 : 3;
				};
			} m_RegRM;
			union
			{
				uint32_t m_Value;
				struct
				{
					uint32_t m_ValidWait : 1;
					uint32_t m_ValidRepe : 1;
					uint32_t m_ValidRepne : 1;
					uint32_t m_Valid66 : 1;
					uint32_t m_ValidRexW : 1;
					uint32_t m_ValidDefault : 1;
					uint32_t m_ValidUnused : 2;

					uint32_t m_IndexRexW : 3;
					uint32_t m_IndexWait : 3;
					uint32_t m_IndexRepe : 3;
					uint32_t m_IndexRepne : 3;
					uint32_t m_Index66 : 3;
					uint32_t m_IndexDefault : 3;
				};
			} m_Prefix;
			union
			{
				uint32_t m_Value;
				struct
				{
					uint32_t m_ValidDefault : 1;
					uint32_t m_Valid0F38 : 1;
					uint32_t m_Valid0F3A : 1;
					uint32_t m_ValidUnused : 1;

					uint32_t m_IndexDefault : 2;
					uint32_t m_Index0F38 : 2;
					uint32_t m_Index0F3A : 2;
				};
			} m_x0F383A;
		};
	};
#pragma pack(pop)

public:
	Redirection() = default;

private:
	Redirection(Type type);
	Redirection(const Instruction& instruction);
	Redirection(std::shared_ptr<Instruction> instruction);

public:
	Redirection(Redirection&&) = default;
	Redirection(const Redirection& copy);

public:
	Redirection& operator=(Redirection&&) = default;
	Redirection& operator=(const Redirection& copy);

public:
	PackageType GetPackageType() const override;
	class BytePackage GetPackage(uint32_t freeSpace) const override;

public:
	static std::shared_ptr<ByteEntry> Insert(std::shared_ptr<ByteEntry> base, std::shared_ptr<Instruction> instruction);
	static std::shared_ptr<ByteEntry> Insert(std::shared_ptr<ByteEntry> base, std::shared_ptr<Instruction> instruction, const std::vector<Entry>& chain);

private:
	static void Sort(std::vector<Entry>& chain);

	static std::shared_ptr<Redirection> Convert(std::shared_ptr<ByteEntry> base, Type type);
	static std::shared_ptr<ByteEntry> ChainForPrefix(std::shared_ptr<ByteEntry> base, Prefix prefix);
	static std::shared_ptr<ByteEntry> Insert(std::shared_ptr<ByteEntry> base, std::shared_ptr<Instruction> instruction, const Entry* chain, uint8_t length);

private:
	Package GetBasePackage(uint32_t freeSpace) const;

private:
	Type m_Type = Type::None;
	std::shared_ptr<ByteEntry> m_Redirects[8];

private:
	friend class Redirection_Unit;
};