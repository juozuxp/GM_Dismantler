#include "FullRedirection.hpp"
#include "InstructionSet.hpp"
#include "Redirection.hpp"
#include "Prefix.hpp"
#include "tools/ArraySize.h"
#include <assert.h>

#define GET_REG(modrrm) ((modrrm >> 3) & ((1 << 3) - 1))
#define GET_MOD(modrrm) ((modrrm >> 6) & ((1 << 2) - 1))
#define GET_MOD_RM(modrrm) (modrrm & ((1 << 3) - 1))

InstructionSet::InstructionSet(const DescriptorSet& descriptors)
{
	CreateCoreBytes();

	for (const Descriptor& descriptor : descriptors)
	{
		m_Types[descriptor.GetName()] = 0;
	}

	uint32_t i = 1;
	for (const std::pair<std::string, uint32_t>& type : m_Types)
	{
		m_Types[type.first] = i++;
	}

	for (const Descriptor& descriptor : descriptors)
	{
		Insert(descriptor);
	}
}

BytePackage InstructionSet::GetPackage() const
{
	BytePackage package = BytePackage();

	uint32_t freeSpace = ARRAY_SIZE(m_Bytes);
	for (uint32_t i = 0; i < ARRAY_SIZE(m_Bytes); i++)
	{
		if (!m_Bytes[i])
		{
			package.AddChild(BytePackage());
			continue;
		}

		BytePackage entryPackage = m_Bytes[i]->GetPackage(freeSpace);

		freeSpace += entryPackage.GetCount() - 1;

		package.AddChild(entryPackage);
	}

	return package;
}

std::vector<std::string> InstructionSet::GetTypes() const
{
	std::vector<std::string> types;

	types.resize(m_Types.size() + 1);

	types[0] = "invalid";
	for (const std::pair<std::string, uint32_t>& type : m_Types)
	{
		types[type.second] = type.first;
	}

	return types;
}

void InstructionSet::Insert(const Descriptor& descriptor)
{
	std::vector<Redirection::Entry> redirections;

	if (descriptor.GetUpperOperand() != static_cast<uint8_t>(~0))
	{
		redirections.push_back(Redirection::Entry(Redirection::Type::Reg, descriptor.GetUpperOperand()));
	}

	bool regOnly = false;
	for (const DescriptorOperand& operand : descriptor.GetOperands())
	{
		if (operand.GetTypeMask().m_Type == DescriptorOperand::Type::mem)
		{
			regOnly = false;

			redirections.push_back(Redirection::Entry::Entry(Redirection::Type::Mod, 0));
			redirections.push_back(Redirection::Entry::Entry(Redirection::Type::Mod, 1));
			redirections.push_back(Redirection::Entry::Entry(Redirection::Type::Mod, 2));
			break;
		}
		else if (operand.GetTypeMask().m_Type == DescriptorOperand::Type::reg && !operand.GetFlagMask().m_Constant)
		{
			regOnly = true;
		}
		else if (operand.GetTypeMask().m_Type == DescriptorOperand::Type::modrm)
		{
			regOnly = false;
			break;
		}
	}

	if (regOnly)
	{
		redirections.push_back(Redirection::Entry::Entry(Redirection::Type::Mod, 3));
	}

	std::shared_ptr<FullRedirection> redirect;

	const std::vector<uint8_t>& bytes = descriptor.GetBytes();
	for (uint8_t i = 0; i < bytes.size(); i++)
	{
		std::shared_ptr<ByteEntry>& entry = redirect ? redirect->Get(bytes[i]) : m_Bytes[bytes[i]];

		if (!entry)
		{
			if (i < (bytes.size() - 1))
			{
				redirections.push_back(Redirection::Entry::Entry(Redirection::Type::Mod, GET_MOD(bytes[i + 1])));
				redirections.push_back(Redirection::Entry::Entry(Redirection::Type::Reg, GET_REG(bytes[i + 1])));
				redirections.push_back(Redirection::Entry::Entry(Redirection::Type::Mem, GET_MOD_RM(bytes[i + 1])));
			}

			entry = Redirection::Insert(entry, std::make_shared<Instruction>(descriptor, m_Types[descriptor.GetName()]), redirections);
			break;
		}

		ByteEntry::PackageType type = entry->GetPackageType();

		if (type == ByteEntry::PackageType::Instruction ||
			type == ByteEntry::PackageType::Redirection)
		{
			if (i < (bytes.size() - 1))
			{
				redirections.push_back(Redirection::Entry::Entry(Redirection::Type::Mod, GET_MOD(bytes[i + 1])));
				redirections.push_back(Redirection::Entry::Entry(Redirection::Type::Reg, GET_REG(bytes[i + 1])));
				redirections.push_back(Redirection::Entry::Entry(Redirection::Type::Mem, GET_MOD_RM(bytes[i + 1])));
			}

			entry = Redirection::Insert(entry, std::make_shared<Instruction>(descriptor, m_Types[descriptor.GetName()]), redirections);
			break;
		}

		if (type == ByteEntry::PackageType::Prefix)
		{
			if (i == (bytes.size() - 1))
			{
				std::shared_ptr<Prefix> prefix = std::reinterpret_pointer_cast<Prefix>(entry);

				assert(prefix->GetRedirectPrefix() != Redirection::Prefix::Default);
				assert(prefix->GetRedirectPrefix() == Redirection::Prefix::Wait); // only known instance for now
				assert(redirections.size() == 0);
				assert(!prefix->IsStandAlone());

				prefix->SetStandAlone(m_Types[descriptor.GetName()]);
			}

			redirections.push_back(Redirection::Entry::Entry(std::reinterpret_pointer_cast<Prefix>(entry)->GetRedirectPrefix()));
		}

		else if (type == ByteEntry::PackageType::FullRedirection)
		{
			redirect = std::reinterpret_pointer_cast<FullRedirection>(entry);

			if (i < (bytes.size() - 1))
			{
				if (bytes[i + 1] == 0x38)
				{
					i++;
					redirections.push_back(Redirection::Entry::Entry(Redirection::X0F383A::x0F38));
					continue;
				}
				else if (bytes[i + 1] == 0x3A)
				{
					i++;
					redirections.push_back(Redirection::Entry::Entry(Redirection::X0F383A::x0F3A));
					continue;
				}
			}
		}
		else
		{
			assert(false);
		}
	}
}

void InstructionSet::CreateCoreBytes()
{
	std::shared_ptr<FullRedirection> subSet = std::make_shared<FullRedirection>();

	m_Bytes[0x0F] = subSet;

	m_Bytes[0x26] = std::make_shared<Prefix>(Prefix::Type_ES);
	m_Bytes[0x2E] = std::make_shared<Prefix>(Prefix::Type_CS);
	m_Bytes[0x36] = std::make_shared<Prefix>(Prefix::Type_SS);
	m_Bytes[0x3E] = std::make_shared<Prefix>(Prefix::Type_DS);

	m_Bytes[0x40] = std::make_shared<Prefix>(Prefix::Type_Rex);

	m_Bytes[0x41] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexB);
	m_Bytes[0x42] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexX);
	m_Bytes[0x43] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexB | Prefix::Type_RexX);

	m_Bytes[0x44] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexR);
	m_Bytes[0x45] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexR | Prefix::Type_RexB);
	m_Bytes[0x46] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexR | Prefix::Type_RexX);
	m_Bytes[0x47] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexR | Prefix::Type_RexX | Prefix::Type_RexB);

	m_Bytes[0x48] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexW);
	m_Bytes[0x49] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexW | Prefix::Type_RexB);
	m_Bytes[0x4A] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexW | Prefix::Type_RexX);
	m_Bytes[0x4B] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexW | Prefix::Type_RexX | Prefix::Type_RexB);
	m_Bytes[0x4C] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexW | Prefix::Type_RexR);
	m_Bytes[0x4D] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexW | Prefix::Type_RexR | Prefix::Type_RexB);
	m_Bytes[0x4E] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexW | Prefix::Type_RexR | Prefix::Type_RexX);
	m_Bytes[0x4F] = std::make_shared<Prefix>(Prefix::Type_Rex | Prefix::Type_RexW | Prefix::Type_RexR | Prefix::Type_RexX | Prefix::Type_RexB);

	m_Bytes[0x64] = std::make_shared<Prefix>(Prefix::Type_FS);
	m_Bytes[0x65] = std::make_shared<Prefix>(Prefix::Type_GS);
	m_Bytes[0x66] = std::make_shared<Prefix>(Prefix::Type_x66);
	m_Bytes[0x67] = std::make_shared<Prefix>(Prefix::Type_x67);

	m_Bytes[0x9B] = std::make_shared<Prefix>(Prefix::Type_Wait);

	m_Bytes[0xF0] = std::make_shared<Prefix>(Prefix::Type_Lock);

	m_Bytes[0xF2] = std::make_shared<Prefix>(Prefix::Type_Repne);
	m_Bytes[0xF3] = std::make_shared<Prefix>(Prefix::Type_Repe);
}