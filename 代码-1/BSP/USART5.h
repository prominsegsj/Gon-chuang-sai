#ifndef __USART5_H__
#define __USART5_H__

void UART5_Init(uint32_t Bound);
void Serial_SendByte_UART5(uint8_t Byte);
void Serial_SendString_UART5(char *String);
void Serial_SendArray_UART5(uint8_t *Array, uint16_t Length);
uint32_t Serial_Pow_UART5(uint32_t X, uint32_t Y);
void Serial_SendNumber_UART5(uint32_t Number, uint8_t Length);

#endif
