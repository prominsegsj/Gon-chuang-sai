#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include <stdio.h>
#include <stdarg.h>
#include "USART2.h"
#include "Buzzer.h"
#include "USART5.h"
#include "OLED.h"

uint8_t Arry[10]={0};
uint8_t Temp=0;
uint8_t rx_Flag=0;
uint8_t Servo_Flag=1;

uint8_t LobotTxBuf[128];  //Ҫ�������ݰ����ݴ�����

#define GET_LOW_BYTE(A) ((uint8_t)(A))
#define GET_HIGH_BYTE(A) ((uint8_t)((A) >> 8))

#define FRAME_HEADER 0x55             //֡ͷ
#define CMD_ACTION_GROUP_RUN 0x06     //���ж�����ָ��

void USART2_Init(uint32_t Bound)
{
	/*����ʱ��*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//USART2ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	  //GPIOAʱ��
	
	/*GPIO��ʼ������*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//PA2ΪTX
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//PA3ΪRX
	
	/*USART��ʼ������*/
	USART_InitTypeDef USART_InitStructure;					
	USART_InitStructure.USART_BaudRate = Bound;				              //����������
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //���ṩ����  
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//�շ�ģʽ
	USART_InitStructure.USART_Parity = USART_Parity_No;		          //��У��λ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	        //1��ֹͣλ
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;     //�ֽڳ���Ϊ8bit
	USART_Init(USART2, &USART_InitStructure);				                //��ֵ�ṹ�壬��ʼ������USART2
	
	/*��������ж�*/
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);			
	
	/*NVIC���ȼ��ּ�*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//NVIC����2
	
	/*NVIC�жϳ�ʼ������*/
	NVIC_InitTypeDef NVIC_InitStructure;					
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;		  //�ж�ͨ��ָ��ΪUSART3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //ȷ��ʹ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;		//�����ȼ�Ϊ3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		      //�����ȼ�Ϊ3
	NVIC_Init(&NVIC_InitStructure);							          
	
	/*USARTʹ��*/
	USART_Cmd(USART2, ENABLE);								//ʹ��USART3	
}

//����һ���ֽ�
void Serial_SendByte_USART2(uint8_t Byte)
{
	USART_SendData(USART2, Byte);	                                 //����Byte����	
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);  //�ȴ��������	
}

//����һ���ַ���
void Serial_SendString_USART2(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)//���������ַ���ֹͣ����
	{
		Serial_SendByte_USART2(String[i]);		
	}
}

//����һ������
void Serial_SendArray_USART2(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)		
	{
		USART_SendData(USART2,Array[i]);
		Delay_ms(50);
	}
}

//�η�����
uint32_t Serial_Pow_USART2(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//���ó�ʼ���Ϊ1
	while (Y --)			//ִ��Y��
	{
		Result *= X;		//��X�۴ε����
	}
	return Result;
}

//���ڷ�������
void Serial_SendNumber_USART2(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		
	{
		Serial_SendByte_USART2(Number / Serial_Pow_USART2(10, Length - i - 1) % 10 + '0');	//���η���ÿλ����
	}
}

//����ö����԰�ķ������麯��
void Usart2WriteBuf(uint8_t *buf, uint8_t len)
{
	while (len--) {
		while ((USART2->SR & 0x40) == 0);
		USART_SendData(USART2,*buf++);
	}
}

//���ƶ�����ж����麯��
void runActionGroup(uint8_t numOfAction, uint16_t Times)
{
	while(Servo_Flag==0);
	LobotTxBuf[0] = LobotTxBuf[1] = FRAME_HEADER;  //��������֡ͷ
	LobotTxBuf[2] = 5;                      //���ݿ���λ��
	LobotTxBuf[3] = CMD_ACTION_GROUP_RUN;   //���ݿ���ִ�ж�����ָ��
	LobotTxBuf[4] = numOfAction;            //ִ��Ҫ���ƵĶ�����
	LobotTxBuf[5] = GET_LOW_BYTE(Times);    //ִ�иö�����Ĵ����Ͱ�λ
	LobotTxBuf[6] = GET_HIGH_BYTE(Times);   //ִ�иö�����Ĵ����߰�λ

	Usart2WriteBuf(LobotTxBuf, 7);          //����
	Servo_Flag=0;
}

/******** ����2 �жϷ����� ***********/
void USART2_IRQHandler(void)
{	
	if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)//�ж��ǲ���������жϷ���
	{
		uint8_t Rx_Data=USART_ReceiveData(USART2);
		switch(rx_Flag)
		{
			case 0:
			{
				if(Rx_Data==0x55)
				{
					Arry[Temp]=Rx_Data;
          if(Temp==1)
					{
						 rx_Flag=1;
					}
					Temp++;						
				}
			}break;
			case 1:
			{
				Arry[Temp]=Rx_Data;
				if(Temp==3)
				{
					 rx_Flag=2;
				}
				Temp++;					
			}break;
			case 2:
			{
				if(Arry[3]==0x08)
				{
					Servo_Flag=1;
				}
				Temp=0;
				rx_Flag=0;
			}break;
		}					
		USART_ClearITPendingBit(USART2, USART_IT_RXNE); //�Ѿ�����������־λ 
	}  
}
