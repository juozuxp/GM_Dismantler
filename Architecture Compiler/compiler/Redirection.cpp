#include "Redirection.hpp"
#include "Instruction.hpp"
#include "BytePackage.hpp"
#include "tools/ArraySize.h"
#include <assert.h>

Redirection::Entry::Entry(Prefix prefix) : 
	m_Type(Type::Prefix), m_Prefix(prefix)
{
}

Redirection::Entry::Entry(X0F383A x0F383A) :
	m_Type(Type::X0F383A), m_X0F383A(x0F383A)
{
}

Redirection::Entry::Entry(Type type, uint8_t index) : 
	m_Type(type), m_Index(index)
{
}

Redirection::Redirection(Type type) : 
	m_Type(type)
{
}

Redirection::Redirection(const Instruction& instruction) : 
	Redirection(Type::None)
{
	m_Redirects[0] = std::make_shared<Instruction>(instruction);
}

Redirection::Redirection(std::shared_ptr<Instruction> instruction) :
	Redirection(Type::None)
{
	m_Redirects[0] = instruction;
}

Redirection::Redirection(const Redirection& copy)
{
	operator=(copy);
}

Redirection& Redirection::operator=(const Redirection& copy)
{
	for (uint8_t i = 0; i < ARRAY_SIZE(m_Redirects); i++)
	{
		if (!copy.m_Redirects[i])
		{
			continue;
		}

		if (copy.m_Redirects[i]->GetPackageType() == PackageType::Redirection)
		{
			m_Redirects[i] = std::make_shared<Redirection>(Redirection(*std::reinterpret_pointer_cast<Redirection>(copy.m_Redirects[i])));
			continue;
		}

		m_Redirects[i] = copy.m_Redirects[i];
	}

	m_Type = copy.m_Type;
	return *this;
}

Redirection::PackageType Redirection::GetPackageType() const
{
	return PackageType::Redirection;
}

BytePackage Redirection::GetPackage(uint32_t freeSpace) const
{
	BytePackage package = BytePackage(GetBasePackage(freeSpace));

	uint8_t count = m_Type == Type::Prefix ? static_cast<uint8_t>(Prefix::ArrayMAX) : (m_Type == Type::X0F383A ? static_cast<uint8_t>(X0F383A::ArrayMAX) : (m_Type == Type::Mod ? 4 : ARRAY_SIZE(m_Redirects)));
	for (uint8_t i = 0; i < count; i++)
	{
		if (m_Redirects[i])
		{
			freeSpace++;
		}
	}

	for (uint8_t i = 0; i < count; i++)
	{
		if (!m_Redirects[i])
		{
			continue;
		}

		BytePackage redirectPackage = m_Redirects[i]->GetPackage(freeSpace);

		freeSpace += redirectPackage.GetCount() - 1;

		package.AddChild(redirectPackage);
	}

	return package;
}

Redirection::Package Redirection::GetBasePackage(uint32_t freeSpace) const
{
	Package package = {};

	package.m_Type = m_Type;
	package.m_BaseIndex = freeSpace;

	uint8_t index = 0;
	switch (m_Type)
	{
	case Type::Prefix:
	{
		for (uint8_t i = 0; i < static_cast<uint8_t>(Prefix::ArrayMAX); i++)
		{
			if (m_Redirects[i])
			{
				package.m_Prefix.m_Value |= (1 << i) | (index << ((i * 3) + 8)); // every 3 bits from 8 we insert an index
				index++;
			}
		}
	} break;
	case Type::X0F383A:
	{
		for (uint8_t i = 0; i < static_cast<uint8_t>(Prefix::ArrayMAX); i++)
		{
			if (m_Redirects[i])
			{
				package.m_x0F383A.m_Value |= (1 << i) | (index << ((i * 2) + 4)); // every 2 bits from 4 we insert an index
				index++;
			}
		}
	} break;
	case Type::Mod:
	{
		for (uint8_t i = 0; i < 4; i++)
		{
			if (m_Redirects[i])
			{
				package.m_Mod.m_Value |= (1 << i) | (index << ((i * 2) + 4)); // every 2 bits from 4 we insert an index
				index++;
			}
		}
	} break;
	default:
	{
		for (uint8_t i = 0; i < ARRAY_SIZE(m_Redirects); i++)
		{
			if (m_Redirects[i])
			{
				package.m_RegRM.m_Value |= (1 << i) | (index << ((i * 3) + 8)); // every 3 bits from 8 we insert an index
				index++;
			}
		}
	} break;
	}

	return package;
}

std::shared_ptr<ByteEntry> Redirection::Insert(std::shared_ptr<ByteEntry> base, std::shared_ptr<Instruction> instruction)
{
	if (!base)
	{
		return instruction;
	}

	assert(base->GetPackageType() == PackageType::Redirection);

	std::shared_ptr<Redirection> redirect = std::reinterpret_pointer_cast<Redirection>(base);

	if (redirect->m_Type == Type::Prefix)
	{
		redirect->m_Redirects[static_cast<uint8_t>(Prefix::Default)] = Insert(redirect->m_Redirects[static_cast<uint8_t>(Prefix::Default)], instruction);
		return redirect;
	}
	
	if (redirect->m_Type == Type::X0F383A)
	{
		redirect->m_Redirects[static_cast<uint8_t>(X0F383A::Default)] = Insert(redirect->m_Redirects[static_cast<uint8_t>(Prefix::Default)], instruction);
		return redirect;
	}
	
	if (redirect->m_Type == Type::Mod)
	{
		for (uint8_t i = 0; i < 4; i++)
		{
			redirect->m_Redirects[i] = Insert(redirect->m_Redirects[i], instruction);
		}

		return redirect;
	}

	for (uint8_t i = 0; i < ARRAY_SIZE(redirect->m_Redirects); i++)
	{
		redirect->m_Redirects[i] = Insert(redirect->m_Redirects[i], instruction);
	}

	return redirect;
}

std::shared_ptr<ByteEntry> Redirection::Insert(std::shared_ptr<ByteEntry> base, std::shared_ptr<Instruction> instruction, const std::vector<Entry>& chain)
{
	std::vector<Entry> copy = std::vector<Entry>(chain);

	Sort(copy);

	return Insert(base, instruction, copy.data(), copy.size());
}

std::shared_ptr<Redirection> Redirection::Convert(std::shared_ptr<ByteEntry> base, Type type)
{
	std::shared_ptr<Redirection> redirect;

	if (base->GetPackageType() != PackageType::Redirection)
	{
		assert(base->GetPackageType() == PackageType::Instruction);

		redirect = std::make_shared<Redirection>(Redirection(std::reinterpret_pointer_cast<Instruction>(base)));
	}
	else
	{
		redirect = std::reinterpret_pointer_cast<Redirection>(base);
	}

	if (type == Type::None || type == redirect->m_Type)
	{
		return redirect;
	}

	if (redirect->m_Type == Type::None)
	{
		assert(redirect->m_Redirects[0]);

		if (type == Type::Prefix)
		{
			redirect->m_Redirects[static_cast<uint8_t>(Prefix::Default)].swap(redirect->m_Redirects[0]);
		}
		else if (type == Type::X0F383A)
		{
			redirect->m_Redirects[static_cast<uint8_t>(X0F383A::Default)].swap(redirect->m_Redirects[0]);
		}
		else if (redirect->m_Type == Type::Mod)
		{
			for (uint8_t i = 1; i < 4; i++)
			{
				redirect->m_Redirects[i] = redirect->m_Redirects[0];
			}
		}
		else
		{
			for (uint8_t i = 1; i < ARRAY_SIZE(redirect->m_Redirects); i++)
			{
				redirect->m_Redirects[i] = redirect->m_Redirects[0];
			}
		}
	}
	else
	{
		assert(redirect->m_Type > type);

		std::shared_ptr<Redirection> newBase = std::make_shared<Redirection>(Redirection(type));
		if (type == Type::Prefix)
		{
			newBase->m_Redirects[static_cast<uint8_t>(Prefix::Default)] = redirect;
			return newBase;
		}

		if (type == Type::X0F383A)
		{
			newBase->m_Redirects[static_cast<uint8_t>(X0F383A::Default)] = redirect;
			return newBase;
		}

		newBase->m_Redirects[0] = redirect;
		if (type == Type::Mod)
		{
			for (uint8_t i = 1; i < 4; i++)
			{
				newBase->m_Redirects[i] = std::make_shared<Redirection>(Redirection(*redirect));
			}

			return newBase;
		}

		for (uint8_t i = 1; i < ARRAY_SIZE(redirect->m_Redirects); i++)
		{
			newBase->m_Redirects[i] = std::make_shared<Redirection>(Redirection(*redirect));
		}

		return newBase;
	}

	redirect->m_Type = type;
	return redirect;
}

std::shared_ptr<ByteEntry> Redirection::ChainForPrefix(std::shared_ptr<ByteEntry> base, Prefix prefix)
{
	if (!base)
	{
		return nullptr;
	}

	assert(prefix < Prefix::Default);

	if (base->GetPackageType() == PackageType::Instruction)
	{
		for (uint8_t entry : std::reinterpret_pointer_cast<Instruction>(base)->GetCompatiblePrefixes())
		{
			if (entry == static_cast<uint8_t>(prefix))
			{
				return base;
			}
		}

		return nullptr;
	}

	assert(base->GetPackageType() == PackageType::Redirection);

	std::shared_ptr<Redirection> redirect = std::reinterpret_pointer_cast<Redirection>(base);

	assert(redirect->m_Type != Type::X0F383A && redirect->m_Type != Type::Prefix);

	if (redirect->m_Type == Type::None)
	{
		return ChainForPrefix(redirect->m_Redirects[0], prefix);
	}

	bool isInserted = false;

	std::shared_ptr<Redirection> result = std::make_shared<Redirection>(Redirection(redirect->m_Type));
	if (redirect->m_Type == Type::Mod)
	{
		for (uint8_t i = 0; i < 4; i++)
		{
			result->m_Redirects[i] = ChainForPrefix(redirect->m_Redirects[i], prefix);
			if (result->m_Redirects[i])
			{
				isInserted = true;
			}
		}
	}
	else
	{
		for (uint8_t i = 0; i < ARRAY_SIZE(redirect->m_Redirects); i++)
		{
			result->m_Redirects[i] = ChainForPrefix(redirect->m_Redirects[i], prefix);
			if (result->m_Redirects[i])
			{
				isInserted = true;
			}
		}
	}

	return isInserted ? result : nullptr;
}

std::shared_ptr<ByteEntry> Redirection::Insert(std::shared_ptr<ByteEntry> base, std::shared_ptr<Instruction> instruction, const Entry* chain, uint8_t length)
{
	if (length == 0)
	{
		return Insert(base, instruction);
	}

	std::shared_ptr<Redirection> redirect;

	if (!base)
	{
		redirect = std::make_shared<Redirection>(Redirection(chain[0].m_Type));
	}
	else
	{
		if (base->GetPackageType() != PackageType::Redirection)
		{
			assert(base->GetPackageType() == PackageType::Instruction);
			redirect = Convert(base, chain[0].m_Type);
		}
		else
		{
			redirect = std::reinterpret_pointer_cast<Redirection>(base);
			if (redirect->m_Type > chain[0].m_Type)
			{
				redirect = Convert(redirect, chain[0].m_Type);
			}
			else if (redirect->m_Type < chain[0].m_Type)
			{
				if (redirect->m_Type == Type::Prefix)
				{
					for (uint8_t entry : instruction->GetCompatiblePrefixes())
					{
						if (!redirect->m_Redirects[entry])
						{
							continue;
						}

						redirect->m_Redirects[entry] = Insert(redirect->m_Redirects[entry], instruction, chain, length);
					}

					redirect->m_Redirects[static_cast<uint8_t>(Prefix::Default)] = Insert(redirect->m_Redirects[static_cast<uint8_t>(Prefix::Default)], instruction, chain, length);
					return redirect;
				}

				if (redirect->m_Type == Type::X0F383A)
				{
					redirect->m_Redirects[static_cast<uint8_t>(X0F383A::Default)] = Insert(redirect->m_Redirects[static_cast<uint8_t>(X0F383A::Default)], instruction, chain, length);
					return redirect;
				}

				if (redirect->m_Type == Type::Mod)
				{
					for (uint8_t i = 0; i < 4; i++)
					{
						redirect->m_Redirects[i] = Insert(redirect->m_Redirects[i], instruction, chain, length);
					}

					return redirect;
				}

				for (uint8_t i = 0; i < ARRAY_SIZE(redirect->m_Redirects); i++)
				{
					redirect->m_Redirects[i] = Insert(redirect->m_Redirects[i], instruction, chain, length);
				}

				return redirect;
			}
		}
	}

	uint8_t next = 0;
	for (uint8_t i = 0; i < length; i++)
	{
		if (chain[i].m_Type != redirect->m_Type)
		{
			next = i;
			break;
		}
	}

	if (next == 0)
	{
		if (redirect->m_Type == Type::Prefix)
		{
			for (uint8_t i = 0; i < length; i++)
			{
				const Entry& entry = chain[i];
				if (redirect->m_Redirects[entry.m_Index])
				{
					continue;
				}

				redirect->m_Redirects[entry.m_Index] = ChainForPrefix(redirect->m_Redirects[static_cast<uint8_t>(Prefix::Default)], entry.m_Prefix);
			}
		}

		for (uint8_t i = 0; i < length; i++)
		{
			const Entry& entry = chain[i];

			redirect->m_Redirects[entry.m_Index] = Insert(redirect->m_Redirects[entry.m_Index], instruction);
		}
	}
	else
	{
		if (redirect->m_Type == Type::Prefix)
		{
			for (uint8_t i = 0; i < next; i++)
			{
				const Entry& entry = chain[i];
				if (redirect->m_Redirects[entry.m_Index])
				{
					continue;
				}

				redirect->m_Redirects[entry.m_Index] = ChainForPrefix(redirect->m_Redirects[static_cast<uint8_t>(Prefix::Default)], entry.m_Prefix);
			}
		}

		for (uint8_t i = 0; i < next; i++)
		{
			const Entry& entry = chain[i];

			redirect->m_Redirects[entry.m_Index] = Insert(redirect->m_Redirects[entry.m_Index], instruction, chain + next, length - next);
		}
	}

	return redirect;
}

void Redirection::Sort(std::vector<Entry>& chain)
{
	for (uint8_t i = 1; i < chain.size(); i++)
	{
		for (uint8_t j = i; j > 0 && chain[j].m_Type < chain[j - 1].m_Type; j--)
		{
			std::swap(chain[j], chain[j - 1]);
		}
	}
}