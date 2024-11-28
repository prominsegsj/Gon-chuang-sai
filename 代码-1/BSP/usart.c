#include "usart.h"
#include "delay.h"
#include <stdio.h>
#include <stdarg.h>
#include "USART2.h"

__IO bool rxFrameFlag = false;
__IO uint8_t rxCmd[FIFO_SIZE] = {0};
__IO uint8_t rxCount = 0;

uint8_t Motor_Flag=0;

/**
	* @brief   USART1中断函数
	* @param   无
	* @retval  无
	*/
void USART1_IRQHandler(void)
{
	__IO uint16_t i = 0;
  uint8_t a[4]={0};
/**********************************************************
***	串口接收中断
**********************************************************/
		if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
		{
			// 未完成一帧数据接收，数据进入缓冲队列
			fifo_enQueue((uint8_t)USART1->DR);

			// 清除串口接收中断
			USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		}

/**********************************************************
***	串口空闲中断
**********************************************************/
		else if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
		{
			// 先读SR再读DR，清除IDLE中断
			USART1->SR; USART1->DR;

			// 提取一帧数据命令
			rxCount = fifo_queueLength(); for(i=0; i < rxCount; i++) { rxCmd[i] = fifo_deQueue(); }

			// 一帧数据接收完成，置位帧标志位
			rxFrameFlag = true;
			
		}		
}

/**
	* @brief   USART发送多个字节
	* @param   无
	* @retval  无
	*/
void usart_SendCmd(__IO uint8_t *cmd, uint8_t len)
{
	__IO uint8_t i = 0;
	
	for(i=0; i < len; i++) { usart_SendByte(cmd[i]); }
}

/**
	* @brief   USART发送一个字节
	* @param   无
	* @retval  无
	*/
void usart_SendByte(uint16_t data)
{
	__IO uint16_t t0 = 0;
	
	USART1->DR = (data & (uint16_t)0x01FF);

	while(!(USART1->SR & USART_FLAG_TXE))
	{
		++t0; if(t0 > 8000)	{	return; }
	}
}

//发送一个字节
void Serial_SendByte_USART1(uint8_t Byte)
{
	USART_SendData(USART1, Byte);	                                 //发送Byte数据	
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);  //等待发送完成	
}

//发送一个字符串
void Serial_SendString_USART1(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)//遇到结束字符串停止发送
	{
		Serial_SendByte_USART1(String[i]);		
	}
}

//发送一个数组
void Serial_SendArray_USART1(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)		
	{
		USART_SendData(USART1,Array[i]);
		Delay_ms(50);
	}
}

//次方函数
uint32_t Serial_Pow_USART1(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//设置初始结果为1
	while (Y --)			//执行Y次
	{
		Result *= X;		//将X累次到结果
	}
	return Result;
}

//串口发送数字
void Serial_SendNumber_USART1(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		
	{
		Serial_SendByte_USART1(Number / Serial_Pow_USART1(10, Length - i - 1) % 10 + '0');	//依次发送每位数据
	}
}

//使用printf需要重定向的底层函数
int fputc(int ch, FILE *f)
{
	Serial_SendByte_USART1(ch);			//将printf重定向到自己的底层发送函数
	return ch;
}

//封装好的Printf函数
void Serial_Printf_USART1(char *format, ...)
{
	char String[100];				
	va_list arg;					
	va_start(arg, format);			
	vsprintf(String, format, arg);	
	va_end(arg);					
	Serial_SendString_USART1(String);		
}
