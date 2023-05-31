#include "lexer/TokenSet.hpp"
#include "parser/DescriptorSet.hpp"

int main()
{
	std::ifstream file = std::ifstream("test.txt");

	TokenSet tokens = TokenSet(file);
	DescriptorSet descriptors = DescriptorSet(tokens);
}