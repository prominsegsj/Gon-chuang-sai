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
函数名：  	UART5_Init
作者：    	
日期：    	2024.08.18
功能：    	初始化uart5
输入参数：	
返回值： 		
修改记录：
**********************************************************/
void UART5_Init(uint32_t Bound)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);	//USART5时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD, ENABLE);	  //GPIOA时钟
	
	/*GPIO初始化配置*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//PA2为TX
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);					//PA3为RX
	
	/*USART初始化配置*/
	USART_InitTypeDef USART_InitStructure;					
	USART_InitStructure.USART_BaudRate = Bound;				              //波特率设置
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //不提供流控  
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//收发模式
	USART_InitStructure.USART_Parity = USART_Parity_No;		          //无校验位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	        //1个停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;     //字节长度为8bit
	USART_Init(UART5, &USART_InitStructure);				                //赋值结构体，初始化配置USART2
	
	/*开启相关中断*/
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);			
	
	/*NVIC优先级分级*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//NVIC分组2
	
	/*NVIC中断初始化配置*/
	NVIC_InitTypeDef NVIC_InitStructure;					
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;		  //中断通道指定为USART3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //确定使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;		//主优先级为3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		      //次优先级为2
	NVIC_Init(&NVIC_InitStructure);							          
	
	/*USART使能*/
	USART_Cmd(UART5, ENABLE);								//使能USART5
	
}
//发送一个字节
void Serial_SendByte_UART5(uint8_t Byte)
{
	USART_SendData(UART5, Byte);	                                 //发送Byte数据	
	while (USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);  //等待发送完成	
}

//发送一个字符串
void Serial_SendString_UART5(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)//遇到结束字符串停止发送
	{
		Serial_SendByte_UART5(String[i]);		
	}
}

//发送一个数组
void Serial_SendArray_UART5(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)		
	{
		USART_SendData(UART5,Array[i]);
		Delay_ms(50);
	}
}

//次方函数
uint32_t Serial_Pow_UART5(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//设置初始结果为1
	while (Y --)			//执行Y次
	{
		Result *= X;		//将X累次到结果
	}
	return Result;
}

//串口发送数字
void Serial_SendNumber_UART5(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		
	{
		Serial_SendByte_UART5(Number / Serial_Pow_UART5(10, Length - i - 1) % 10 + '0');	//依次发送每位数据
	}
}

uint8_t Color_Data[3]={0};
uint8_t D1[3]={0};
uint8_t Ring_Data[2]={0};
uint8_t D2[2]={0};
extern uint8_t Flag_Color;
extern uint8_t Flag_Ring;

/******** 串口5 中断服务函数 ***********/
void UART5_IRQHandler(void)
{
	static uint8_t RxFlag = 0;
	static uint8_t Num=0;
	if(USART_GetITStatus(UART5, USART_IT_RXNE) == SET)//判断是不是真的有中断发生
	{
		//接收数据
		uint8_t RxDate=USART_ReceiveData(UART5);
		
		//状态机接收数据
		if(RxFlag==0)
		{
			 //数据包的包头
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
			//数据包的包尾
			else if(RxDate==END_FLAG)
			{
				RxFlag=3;
			}
		}
		
		if(RxFlag==3)
		{
			//将数据储存下来 并且显示用于调试
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
			
			//状态位清零
			RxFlag=0;
			Num=0;
			
		}
		OLED_ShowHexNum(3,1,RxDate,2);
		USART_ClearITPendingBit(UART5, USART_IT_RXNE); //已经处理就清楚标志位 
	}  
}
