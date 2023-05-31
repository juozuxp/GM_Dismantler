#include "Redirection.hpp"
#include "Instruction.hpp"
#include "ByteEntry.hpp"
#include "tools/ArraySize.h"

Redirection::Redirection()
{
	m_Redirects = std::vector<ByteEntry>();
	m_Redirects.resize(ARRAY_SIZE(m_ValidMask));
}

void Redirection::Insert(const Instruction& instruction, const std::vector<Entry>& chain)
{

}

void Redirection::Insert(const Instruction& instruction, const Entry* chain, uint8_t length)
{

}