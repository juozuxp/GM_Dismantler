#pragma once
#include <stdint.h>
#include <vector>

class Redirection
{
public:
	enum class Type : uint8_t
	{
		Mod,
		Reg,
		Mem,
		Prefix
	};

	enum class Prefix : uint8_t
	{
		None,
		RexW,
		Wait,
		Repe,
		Repne,
		x66,
		x0F38,
		x0F3A
	};

	struct Entry
	{
		Type m_Type;

		union
		{
			Prefix m_Prefix;
			uint8_t m_Index;
		};
	};

#pragma pack(push, 1)
	struct Package
	{
		uint16_t m_BaseIndex;
		union
		{
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
			} m_Mod;
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
			} m_RegRM;
			struct
			{
				uint32_t m_ValidDefault : 1;
				uint32_t m_ValidRexW : 1;
				uint32_t m_ValidWait : 1;
				uint32_t m_ValidRepe : 1;
				uint32_t m_ValidRepne : 1;
				uint32_t m_Valid66 : 1;
				uint32_t m_Valid0F38 : 1;
				uint32_t m_Valid0F3A : 1;

				uint32_t m_IndexDefault : 3;
				uint32_t m_IndexRexW : 3;
				uint32_t m_IndexWait : 3;
				uint32_t m_IndexRepe : 3;
				uint32_t m_IndexRepne : 3;
				uint32_t m_Index66 : 3;
				uint32_t m_Index0F38 : 3;
				uint32_t m_Index0F3A : 3;
			} m_Prefix;
		};
	};
#pragma pack(pop)

public:
	Redirection();

public:
	void Insert(const class Instruction& instruction, const std::vector<Entry>& chain);

private:
	void Insert(const class Instruction& instruction, const Entry* chain, uint8_t length);

private:
	Type m_Type;

	bool m_ValidMask[8];
	std::vector<class ByteEntry> m_Redirects;
};