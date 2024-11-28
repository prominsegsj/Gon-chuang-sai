#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include <stdio.h>
#include <stdarg.h>
#include "USART5.h"
#include "Buzzer.h"
#include "OLED.h"
#include "USART2.h"
#include "OLED.h"
#include "Task_Work.h"

#define HEADER_FLAG_COLOR 0xFF
#define HEADER_FLAG_RING  0xEE
#define END_FLAG          0xFE

/********************************************************
��������  	UART5_Init
���ߣ�    	
���ڣ�    	2024.08.18
���ܣ�    	��ʼ��uart5
���������	
����ֵ�� 		
�޸ļ�¼��
**********************************************************/
void UART5_Init(uint32_t Bound)
{
	/*����ʱ��*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	//USART5ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);	  //GPIOAʱ��
	
	/*GPIO��ʼ������*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//PA2ΪTX
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);					//PA3ΪRX
	
	/*USART��ʼ������*/
	USART_InitTypeDef USART_InitStructure;					
	USART_InitStructure.USART_BaudRate = Bound;				              //����������
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //���ṩ����  
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//�շ�ģʽ
	USART_InitStructure.USART_Parity = USART_Parity_No;		          //��У��λ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	        //1��ֹͣλ
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;     //�ֽڳ���Ϊ8bit
	USART_Init(UART5, &USART_InitStructure);				                //��ֵ�ṹ�壬��ʼ������USART2
	
	/*��������ж�*/
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);			
	
	/*NVIC���ȼ��ּ�*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//NVIC����2
	
	/*NVIC�жϳ�ʼ������*/
	NVIC_InitTypeDef NVIC_InitStructure;					
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;		  //�ж�ͨ��ָ��ΪUSART3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //ȷ��ʹ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;		//�����ȼ�Ϊ3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		      //�����ȼ�Ϊ2
	NVIC_Init(&NVIC_InitStructure);							          
	
	/*USARTʹ��*/
	USART_Cmd(UART5, ENABLE);								//ʹ��USART5
	
}
//����һ���ֽ�
void Serial_SendByte_UART5(uint8_t Byte)
{
	USART_SendData(UART5, Byte);	                                 //����Byte����	
	while (USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);  //�ȴ��������	
}

//����һ���ַ���
void Serial_SendString_UART5(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)//���������ַ���ֹͣ����
	{
		Serial_SendByte_UART5(String[i]);		
	}
}

//����һ������
void Serial_SendArray_UART5(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)		
	{
		USART_SendData(UART5,Array[i]);
		Delay_ms(50);
	}
}

//�η�����
uint32_t Serial_Pow_UART5(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//���ó�ʼ���Ϊ1
	while (Y --)			//ִ��Y��
	{
		Result *= X;		//��X�۴ε����
	}
	return Result;
}

//���ڷ�������
void Serial_SendNumber_UART5(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		
	{
		Serial_SendByte_UART5(Number / Serial_Pow_UART5(10, Length - i - 1) % 10 + '0');	//���η���ÿλ����
	}
}

uint8_t Color_Data[3]={0};
uint8_t D1[3]={0};
uint8_t Ring_Data[2]={0};
uint8_t D2[2]={0};
extern uint8_t Flag_Color;
extern uint8_t Flag_Ring;

/******** ����5 �жϷ����� ***********/
void UART5_IRQHandler(void)
{
	static uint8_t RxFlag = 0;
	static uint8_t Num=0;
	if(USART_GetITStatus(UART5, USART_IT_RXNE) == SET)//�ж��ǲ���������жϷ���
	{
		//��������
		uint8_t RxDate=USART_ReceiveData(UART5);
		
		//״̬����������
		if(RxFlag==0)
		{
			 //���ݰ��İ�ͷ
			 if(RxDate==HEADER_FLAG_COLOR)
			 {
				  RxFlag=1;
			 }
			 else if(RxDate==HEADER_FLAG_RING)
			 {
				  RxFlag=2;
			 }
		}
		
		else if(RxFlag==1||RxFlag==2)
		{
			if(RxDate!=END_FLAG)
			{
				if(RxFlag==1)
				{
					Color_Data[Num]=RxDate;
					Num++;					
				}
				else if(RxFlag==2)
				{
					Ring_Data[Num]=RxDate;
					Num++;										
				}
			}
			//���ݰ��İ�β
			else if(RxDate==END_FLAG)
			{
				RxFlag=3;
			}
		}
		
		if(RxFlag==3)
		{
			//�����ݴ������� ������ʾ���ڵ���
			if(Flag_Color==0)
			{
				for(int i=0;i<3;i++)
				{
					D1[i]=Color_Data[i];
				}				
				OLED_ShowNum(2,1,Color_Data[0],2);
				OLED_ShowNum(2,5,Color_Data[1],2);
				OLED_ShowNum(2,9,Color_Data[2],2);
				Flag_Color=1;
			}
			else if(Flag_Ring==0)
			{
				for(int i=0;i<3;i++)
				{
					D2[i]=Ring_Data[i];
				}				
				OLED_ShowNum(3,5,Ring_Data[0],2);
				OLED_ShowNum(3,9,Ring_Data[1],2);
				Flag_Ring=1;				
			}						
			
			//״̬λ����
			RxFlag=0;
			Num=0;
			
		}
		OLED_ShowHexNum(3,1,RxDate,2);
		USART_ClearITPendingBit(UART5, USART_IT_RXNE); //�Ѿ�����������־λ 
	}  
}
