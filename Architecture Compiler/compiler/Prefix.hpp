#pragma once
class Prefix
{
public:
#ifdef INSTRUCTION_PACKING
#pragma pack(push, INSTRUCTION_PACKING)
#else
#pragma pack(push, 1)
#endif
	struct Package
	{
	};
#pragma pack(pop)
};

