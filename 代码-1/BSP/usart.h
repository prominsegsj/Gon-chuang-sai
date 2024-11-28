#ifndef __USART_H
#define __USART_H

#include "board.h"
#include "fifo.h"

extern __IO bool rxFrameFlag;
extern __IO uint8_t rxCmd[FIFO_SIZE];
extern __IO uint8_t rxCount;

void usart_SendCmd(__IO uint8_t *cmd, uint8_t len);
void usart_SendByte(uint16_t data);

void Serial_SendByte_USART1(uint8_t Byte);
void Serial_SendString_USART1(char *String);
void Serial_SendArray_USART1(uint8_t *Array, uint16_t Length);
uint32_t Serial_Pow_USART1(uint32_t X, uint32_t Y);
void Serial_SendNumber_USART1(uint32_t Number, uint8_t Length);
void Serial_Printf_USART1(char *format, ...);
	
#endif
