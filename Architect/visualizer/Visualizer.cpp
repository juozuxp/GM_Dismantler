#include "Visualizer.hpp"
#include "ArraySize.hpp"

void Visualizer::PrintToConsole(const std::vector<ILInstruction>& instructions)
{
	for (const ILInstruction& instruction : instructions)
	{
		if (instruction.m_Type == Instruction_invalid)
		{
			printf("invalid\n");
			continue;
		}

		printf(TypeToName[instruction.m_Type]);

		for (uint8_t i = 0; i < ARRAY_SIZE(instruction.m_Operands); i++)
		{
			const ILOperand& operand = instruction.m_Operands[i];

			if (operand.m_Type == ILOperandType_None)
			{
				break;
			}

			if (i != 0)
			{
				printf(", ");
			}
			else
			{
				printf(" ");
			}

			switch (operand.m_Type)
			{
			case ILOperandType_Memory:
			{
				printf("[");

				if (operand.m_Memory.m_Base != IL_INVALID_REGISTER)
				{
					printf("r%u", operand.m_Memory.m_Base);
				}

				if (operand.m_Memory.m_Index != IL_INVALID_REGISTER)
				{
					if (operand.m_Memory.m_Base != IL_INVALID_REGISTER)
					{
						printf(" + ");
					}

					printf("r%u", operand.m_Memory.m_Index);

					switch (operand.m_Memory.m_Scale)
					{
					case ILMemoryScaler_2:
					{
						printf(" * 2");
					} break;
					case ILMemoryScaler_4:
					{
						printf(" * 4");
					} break;
					case ILMemoryScaler_8:
					{
						printf(" * 8");
					} break;
					}
				}

				if (operand.m_Memory.m_Offset != 0)
				{
					bool sign = operand.m_Memory.m_Offset < 0;
					uint32_t value = sign ? -operand.m_Memory.m_Offset : operand.m_Memory.m_Offset;

					printf(" %c %X]", sign ? '-' : '+', value);
				}
				else
				{
					printf("]");
				}
			} break;
			case ILOperandType_Register:
			{
				printf("r%u", operand.m_Register.m_Base);

				if (operand.m_Register.m_BaseHigh)
				{
					printf("h");
				}
			} break;
			case ILOperandType_Value:
			{
				printf("%llX", operand.m_Value);
			} break;
			case ILOperandType_ValueRelative:
			{
				uint64_t value = 0;
				bool sign = false;

				switch (operand.m_Scale)
				{
				case ILOperandScale_8:
				{
					int8_t convert = static_cast<int8_t>(operand.m_Value);

					sign = convert < 0;
					value = sign ? -convert : convert;
				} break;
				case ILOperandScale_16:
				{
					int16_t convert = static_cast<int16_t>(operand.m_Value);
					
					sign = convert < 0;
					value = sign ? -convert : convert;
				} break;
				case ILOperandScale_32:
				{
					int32_t convert = static_cast<int32_t>(operand.m_Value);
					
					sign = convert < 0;
					value = sign ? -convert : convert;
				} break;
				}

				printf("rip %c %X", sign ? '-' : '+', value);
			} break;
			case ILOperandType_MemoryRelative:
			{
				uint64_t value = 0;
				bool sign = false;

				switch (operand.m_Scale)
				{
				case ILOperandScale_8:
				{
					int8_t convert = static_cast<int8_t>(operand.m_Value);

					sign = convert < 0;
					value = sign ? -convert : convert;
				} break;
				case ILOperandScale_16:
				{
					int16_t convert = static_cast<int16_t>(operand.m_Value);

					sign = convert < 0;
					value = sign ? -convert : convert;
				} break;
				case ILOperandScale_32:
				{
					int32_t convert = static_cast<int32_t>(operand.m_Value);

					sign = convert < 0;
					value = sign ? -convert : convert;
				} break;
				}

				printf("[rip %c %X]", sign ? '-' : '+', value);
			} break;
			case ILOperandType_MemoryAbsolute:
			{
				printf("[%llX]", operand.m_Value);
			} break;
			}
		}

		printf("\n");
	}
}