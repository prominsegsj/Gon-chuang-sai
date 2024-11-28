#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include <stdio.h>
#include <stdarg.h>

void USART3_Init(uint32_t Bound)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//USART3时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	  //GPIOB时钟
	
	/*GPIO初始化配置*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//PB10为TX
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//PB11为RX
	
	/*USART初始化配置*/
	USART_InitTypeDef USART_InitStructure;					
	USART_InitStructure.USART_BaudRate = Bound;				              //波特率设置
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //不提供流控  
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//收发模式
	USART_InitStructure.USART_Parity = USART_Parity_No;		          //无校验位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	        //1个停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;     //字节长度为8bit
	USART_Init(USART3, &USART_InitStructure);				                //赋值结构体，初始化配置USART3
	
	/*开启相关中断*/
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);			
	
	/*NVIC优先级分级*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//NVIC分组2
	
	/*NVIC中断初始化配置*/
	NVIC_InitTypeDef NVIC_InitStructure;					
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;		  //中断通道指定为USART3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //确定使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;		//主优先级为3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		      //次优先级为3
	NVIC_Init(&NVIC_InitStructure);							          
	
	/*USART使能*/
	USART_Cmd(USART3, ENABLE);								//使能USART3
	
}

//发送一个字节
void Serial_SendByte_USART3(uint8_t Byte)
{
	USART_SendData(USART3, Byte);	                                 //发送Byte数据	
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);  //等待发送完成	
}

//发送一个字符串
void Serial_SendString_USART3(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)//遇到结束字符串停止发送
	{
		Serial_SendByte_USART3(String[i]);		
	}
}

//发送一个数组
void Serial_SendArray_USART3(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)		
	{
		USART_SendData(USART3,Array[i]);
		Delay_ms(50);
	}
}

//次方函数
uint32_t Serial_Pow_USART3(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//设置初始结果为1
	while (Y --)			//执行Y次
	{
		Result *= X;		//将X累次到结果
	}
	return Result;
}

//串口发送数字
void Serial_SendNumber_USART3(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		
	{
		Serial_SendByte_USART3(Number / Serial_Pow_USART3(10, Length - i - 1) % 10 + '0');	//依次发送每位数据
	}
}

////使用printf需要重定向的底层函数
//int fputc(int ch, FILE *f)
//{
//	Serial_SendByte_USART3(ch);			//将printf重定向到自己的底层发送函数
//	return ch;
//}

////封装好的Printf函数
//void Serial_Printf(char *format, ...)
//{
//	char String[100];				
//	va_list arg;					
//	va_start(arg, format);			
//	vsprintf(String, format, arg);	
//	va_end(arg);					
//	Serial_SendString_USART3(String);		
//}

//专门服务于扫码模块  仅用于本次工创赛
int Make_Date(uint8_t *Date,uint8_t Lencth)
{
	int DATA=0;
	int Temp_DATA=0;
	for(int a=0;a<Lencth;a++)
	{
		 switch(Date[a])
		 {
			 case 49:Temp_DATA=1;break;
			 case 50:Temp_DATA=2;break;
			 case 51:Temp_DATA=3;break;			 
		 }
		 DATA=DATA*10+Temp_DATA;
	}
	return DATA;
}

void Set_ON(void)
{
	uint8_t abc[9]={0x7E,0x00,0x08,0x01,0x00,0x02,0x01,0xAB,0xCD};
	Serial_SendArray_USART3(abc,9);	
}

int Get_Flag=0;
int DATA1;
int DATA2;
uint8_t DATA_Temp[10]={0};
uint8_t data1[3]={0};
uint8_t data2[3]={0};
uint8_t Num=0;

//中断服务函数
void USART3_IRQHandler(void)
{
	static uint8_t RxFlag = 0;			
	if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)//判断是不是真的有中断发生
	{
		//接收数据
		uint8_t RxDate=USART_ReceiveData(USART3);
//		USART_SendData(USART3,USART_ReceiveData(USART3));
		//状态机接收数据
		if(RxFlag==0)
		{
			 //数据包的包头
			 if(RxDate=='?')
			 {
				  RxFlag=1;
			 }
		}
		else if(RxFlag==1)
		{
			if(RxDate!='@')
			{
				DATA_Temp[Num]=RxDate;
				Num++;
			}
			//数据包的包尾
			else if(RxDate=='@')
			{
				Get_Flag=1;
				RxFlag=2;
			}
		}
		else if(RxFlag==2)
		{
			//将两组数据保存下来 除去中间的+
			for(int i=0;i<3;i++)
			{
				data1[i]=DATA_Temp[i];
			}
			for(int j=0;j<3;j++)
			{
				data2[j]=DATA_Temp[j+4];
			}
			
			//数据转换保存
			DATA1=Make_Date(data1,3);
			DATA2=Make_Date(data2,3);
			
			//状态位清零
			RxFlag=0;
			Num=0;
			
		}
				
		USART_ClearITPendingBit(USART3, USART_IT_RXNE); //已经处理就清楚标志位 
	}  
}
