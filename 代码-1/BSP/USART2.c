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

uint8_t LobotTxBuf[128];  //要发送数据包的暂存数组

#define GET_LOW_BYTE(A) ((uint8_t)(A))
#define GET_HIGH_BYTE(A) ((uint8_t)((A) >> 8))

#define FRAME_HEADER 0x55             //帧头
#define CMD_ACTION_GROUP_RUN 0x06     //运行动作组指令

void USART2_Init(uint32_t Bound)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//USART2时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	  //GPIOA时钟
	
	/*GPIO初始化配置*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//PA2为TX
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//PA3为RX
	
	/*USART初始化配置*/
	USART_InitTypeDef USART_InitStructure;					
	USART_InitStructure.USART_BaudRate = Bound;				              //波特率设置
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //不提供流控  
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//收发模式
	USART_InitStructure.USART_Parity = USART_Parity_No;		          //无校验位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	        //1个停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;     //字节长度为8bit
	USART_Init(USART2, &USART_InitStructure);				                //赋值结构体，初始化配置USART2
	
	/*开启相关中断*/
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);			
	
	/*NVIC优先级分级*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//NVIC分组2
	
	/*NVIC中断初始化配置*/
	NVIC_InitTypeDef NVIC_InitStructure;					
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;		  //中断通道指定为USART3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //确定使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;		//主优先级为3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		      //次优先级为3
	NVIC_Init(&NVIC_InitStructure);							          
	
	/*USART使能*/
	USART_Cmd(USART2, ENABLE);								//使能USART3	
}

//发送一个字节
void Serial_SendByte_USART2(uint8_t Byte)
{
	USART_SendData(USART2, Byte);	                                 //发送Byte数据	
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);  //等待发送完成	
}

//发送一个字符串
void Serial_SendString_USART2(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)//遇到结束字符串停止发送
	{
		Serial_SendByte_USART2(String[i]);		
	}
}

//发送一个数组
void Serial_SendArray_USART2(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)		
	{
		USART_SendData(USART2,Array[i]);
		Delay_ms(50);
	}
}

//次方函数
uint32_t Serial_Pow_USART2(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//设置初始结果为1
	while (Y --)			//执行Y次
	{
		Result *= X;		//将X累次到结果
	}
	return Result;
}

//串口发送数字
void Serial_SendNumber_USART2(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		
	{
		Serial_SendByte_USART2(Number / Serial_Pow_USART2(10, Length - i - 1) % 10 + '0');	//依次发送每位数据
	}
}

//服务幻尔调试板的发送数组函数
void Usart2WriteBuf(uint8_t *buf, uint8_t len)
{
	while (len--) {
		while ((USART2->SR & 0x40) == 0);
		USART_SendData(USART2,*buf++);
	}
}

//控制舵机运行动作组函数
void runActionGroup(uint8_t numOfAction, uint16_t Times)
{
	while(Servo_Flag==0);
	LobotTxBuf[0] = LobotTxBuf[1] = FRAME_HEADER;  //发送数据帧头
	LobotTxBuf[2] = 5;                      //数据控制位数
	LobotTxBuf[3] = CMD_ACTION_GROUP_RUN;   //数据控制执行动作组指令
	LobotTxBuf[4] = numOfAction;            //执行要控制的动作组
	LobotTxBuf[5] = GET_LOW_BYTE(Times);    //执行该动作组的次数低八位
	LobotTxBuf[6] = GET_HIGH_BYTE(Times);   //执行该动作组的次数高八位

	Usart2WriteBuf(LobotTxBuf, 7);          //发送
	Servo_Flag=0;
}

/******** 串口2 中断服务函数 ***********/
void USART2_IRQHandler(void)
{	
	if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)//判断是不是真的有中断发生
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
		USART_ClearITPendingBit(USART2, USART_IT_RXNE); //已经处理就清楚标志位 
	}  
}
