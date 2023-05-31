#pragma once
#include <stdint.h>

class Redirection
{
public:
#ifdef INSTRUCTION_PACKING
#pragma pack(push, INSTRUCTION_PACKING)
#else
#pragma pack(push, 1)
#endif
	struct Package
	{
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
				uint32_t m_ValidRexW : 1;
				uint32_t m_ValidWait : 1;
				uint32_t m_Valid66 : 1;
				uint32_t m_ValidRepe : 1;
				uint32_t m_ValidRepne : 1;

				uint32_t m_IndexRexW : 3;
				uint32_t m_IndexWait : 3;
				uint32_t m_Index66 : 3;
				uint32_t m_IndexRepe : 3;
				uint32_t m_IndexRepne : 3;
			} m_Prefix;
		};
	};
#pragma pack(pop)


};