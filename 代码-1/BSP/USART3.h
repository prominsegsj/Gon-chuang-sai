#ifndef __USART3_H__
#define __USART3_H__

void USART3_Init(uint32_t Bound);
void Serial_SendByte_USART3(uint8_t Byte);
void Serial_SendString_USART3(char *String);
void Serial_SendArray_USART3(uint8_t *Array, uint16_t Length);
uint32_t Serial_Pow_USART3(uint32_t X, uint32_t Y);
void Serial_SendNumber_USART3(uint32_t Number, uint8_t Length);
int Make_Date(uint8_t *Date,uint8_t Lencth);
void Set_ON(void);
	
#endif
