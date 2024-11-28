#ifndef __USART2_H__
#define __USART2_H__

void USART2_Init(uint32_t Bound);
void Serial_SendByte_USART2(uint8_t Byte);
void Serial_SendString_USART2(char *String);
void Serial_SendArray_USART2(uint8_t *Array, uint16_t Length);
uint32_t Serial_Pow_USART2(uint32_t X, uint32_t Y);
void Serial_SendNumber_USART2(uint32_t Number, uint8_t Length);
void Usart2WriteBuf(uint8_t *buf, uint8_t len);
void runActionGroup(uint8_t numOfAction, uint16_t Times);

#endif