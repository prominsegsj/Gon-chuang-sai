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
	* @brief   USART1�жϺ���
	* @param   ��
	* @retval  ��
	*/
void USART1_IRQHandler(void)
{
	__IO uint16_t i = 0;
  uint8_t a[4]={0};
/**********************************************************
***	���ڽ����ж�
**********************************************************/
		if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
		{
			// δ���һ֡���ݽ��գ����ݽ��뻺�����
			fifo_enQueue((uint8_t)USART1->DR);

			// ������ڽ����ж�
			USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		}

/**********************************************************
***	���ڿ����ж�
**********************************************************/
		else if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
		{
			// �ȶ�SR�ٶ�DR�����IDLE�ж�
			USART1->SR; USART1->DR;

			// ��ȡһ֡��������
			rxCount = fifo_queueLength(); for(i=0; i < rxCount; i++) { rxCmd[i] = fifo_deQueue(); }

			// һ֡���ݽ�����ɣ���λ֡��־λ
			rxFrameFlag = true;
			
		}		
}

/**
	* @brief   USART���Ͷ���ֽ�
	* @param   ��
	* @retval  ��
	*/
void usart_SendCmd(__IO uint8_t *cmd, uint8_t len)
{
	__IO uint8_t i = 0;
	
	for(i=0; i < len; i++) { usart_SendByte(cmd[i]); }
}

/**
	* @brief   USART����һ���ֽ�
	* @param   ��
	* @retval  ��
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

//����һ���ֽ�
void Serial_SendByte_USART1(uint8_t Byte)
{
	USART_SendData(USART1, Byte);	                                 //����Byte����	
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);  //�ȴ��������	
}

//����һ���ַ���
void Serial_SendString_USART1(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)//���������ַ���ֹͣ����
	{
		Serial_SendByte_USART1(String[i]);		
	}
}

//����һ������
void Serial_SendArray_USART1(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)		
	{
		USART_SendData(USART1,Array[i]);
		Delay_ms(50);
	}
}

//�η�����
uint32_t Serial_Pow_USART1(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//���ó�ʼ���Ϊ1
	while (Y --)			//ִ��Y��
	{
		Result *= X;		//��X�۴ε����
	}
	return Result;
}

//���ڷ�������
void Serial_SendNumber_USART1(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		
	{
		Serial_SendByte_USART1(Number / Serial_Pow_USART1(10, Length - i - 1) % 10 + '0');	//���η���ÿλ����
	}
}

//ʹ��printf��Ҫ�ض���ĵײ㺯��
int fputc(int ch, FILE *f)
{
	Serial_SendByte_USART1(ch);			//��printf�ض����Լ��ĵײ㷢�ͺ���
	return ch;
}

//��װ�õ�Printf����
void Serial_Printf_USART1(char *format, ...)
{
	char String[100];				
	va_list arg;					
	va_start(arg, format);			
	vsprintf(String, format, arg);	
	va_end(arg);					
	Serial_SendString_USART1(String);		
}
