#pragma once
#include "AsmIndex.hpp"

class AsmInstruction : public AsmIndex
{
public:
	enum class DispImmSize : uint8_t
	{
		none,
		size8,
		size16,
		size32,
		size64
	};

	struct Blob
	{
		Blob() = default;
		Blob(const Index& index);

		uint8_t GetSize() const;

		Prefix m_Prefixes = {};
		struct
		{
			bool m_Valid : 1 = false;

			bool m_HasBase : 1 = false;
			bool m_HasX0F383A : 1 = false;

			bool m_ModRMValid : 1 = false;
			bool m_SIBValid : 1 = false;

			bool m_DispIndex : 1 = false; // m_Disp is set to relative index
		};

		uint8_t m_Base = 0;
		uint8_t m_X0F383A = 0;
		uint8_t m_Core = 0;

		struct
		{
			DispImmSize m_DispSize : 4 = DispImmSize::none;
			DispImmSize m_ImmSize : 4 = DispImmSize::none;
		};

		struct
		{
			uint8_t m_Mem : 3 = 0;
			uint8_t m_Reg : 3 = 0;
			uint8_t m_Mod : 2 = 0;
		} m_ModRM;

		struct
		{
			uint8_t m_Base : 3 = 0;
			uint8_t m_Index : 3 = 0;
			uint8_t m_Scale : 2 = 0;
		} m_SIB;

		int64_t m_Disp = 0;
		int64_t m_Imm = 0;
	};

public:
	AsmInstruction(const Index& index);

public:
	static void Compile(const std::vector<Blob>& blobs, std::vector<uint8_t>& bytes);

public:
	Blob Assemble(const ILInstruction& instruction) const; // Constructs a compilable blob

public:
	std::shared_ptr<const AsmInstruction> GetIndex(const ILInstruction& instruction) const override final;

private:
	Index m_Template;
};