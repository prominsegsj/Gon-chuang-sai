#ifndef __USART4_H__
#define __USART4_H__
 
#include "stm32f10x.h"
  
//外部可调用函数的声明
void UART4_Init(uint32_t __Baud);
void UART4_IRQHandler(void);
void TJC_SendData(uint8_t *data, uint16_t len);

/**
	打印到屏幕串口
*/
void TJCPrintf(const char *cmd, ...);
void initRingBuff(void);
void writeRingBuff(uint8_t data);
void deleteRingBuff(uint16_t size);
uint16_t getRingBuffLenght(void);
uint8_t read1BFromRingBuff(uint16_t position);

#define RINGBUFF_LEN	(500)     //定义最大接收字节数 500

#define usize getRingBuffLenght()
#define code_c() initRingBuff()
#define udelete(x) deleteRingBuff(x)
#define u(x) read1BFromRingBuff(x)

#endif
