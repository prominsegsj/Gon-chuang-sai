#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include <stdio.h>
#include <stdarg.h>

void USART3_Init(uint32_t Bound)
{
	/*����ʱ��*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//USART3ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	  //GPIOBʱ��
	
	/*GPIO��ʼ������*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//PB10ΪTX
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//PB11ΪRX
	
	/*USART��ʼ������*/
	USART_InitTypeDef USART_InitStructure;					
	USART_InitStructure.USART_BaudRate = Bound;				              //����������
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //���ṩ����  
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//�շ�ģʽ
	USART_InitStructure.USART_Parity = USART_Parity_No;		          //��У��λ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	        //1��ֹͣλ
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;     //�ֽڳ���Ϊ8bit
	USART_Init(USART3, &USART_InitStructure);				                //��ֵ�ṹ�壬��ʼ������USART3
	
	/*��������ж�*/
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);			
	
	/*NVIC���ȼ��ּ�*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//NVIC����2
	
	/*NVIC�жϳ�ʼ������*/
	NVIC_InitTypeDef NVIC_InitStructure;					
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;		  //�ж�ͨ��ָ��ΪUSART3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  //ȷ��ʹ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;		//�����ȼ�Ϊ3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		      //�����ȼ�Ϊ3
	NVIC_Init(&NVIC_InitStructure);							          
	
	/*USARTʹ��*/
	USART_Cmd(USART3, ENABLE);								//ʹ��USART3
	
}

//����һ���ֽ�
void Serial_SendByte_USART3(uint8_t Byte)
{
	USART_SendData(USART3, Byte);	                                 //����Byte����	
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);  //�ȴ��������	
}

//����һ���ַ���
void Serial_SendString_USART3(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)//���������ַ���ֹͣ����
	{
		Serial_SendByte_USART3(String[i]);		
	}
}

//����һ������
void Serial_SendArray_USART3(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)		
	{
		USART_SendData(USART3,Array[i]);
		Delay_ms(50);
	}
}

//�η�����
uint32_t Serial_Pow_USART3(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//���ó�ʼ���Ϊ1
	while (Y --)			//ִ��Y��
	{
		Result *= X;		//��X�۴ε����
	}
	return Result;
}

//���ڷ�������
void Serial_SendNumber_USART3(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		
	{
		Serial_SendByte_USART3(Number / Serial_Pow_USART3(10, Length - i - 1) % 10 + '0');	//���η���ÿλ����
	}
}

////ʹ��printf��Ҫ�ض���ĵײ㺯��
//int fputc(int ch, FILE *f)
//{
//	Serial_SendByte_USART3(ch);			//��printf�ض����Լ��ĵײ㷢�ͺ���
//	return ch;
//}

////��װ�õ�Printf����
//void Serial_Printf(char *format, ...)
//{
//	char String[100];				
//	va_list arg;					
//	va_start(arg, format);			
//	vsprintf(String, format, arg);	
//	va_end(arg);					
//	Serial_SendString_USART3(String);		
//}

//ר�ŷ�����ɨ��ģ��  �����ڱ��ι�����
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

//�жϷ�����
void USART3_IRQHandler(void)
{
	static uint8_t RxFlag = 0;			
	if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)//�ж��ǲ���������жϷ���
	{
		//��������
		uint8_t RxDate=USART_ReceiveData(USART3);
//		USART_SendData(USART3,USART_ReceiveData(USART3));
		//״̬����������
		if(RxFlag==0)
		{
			 //���ݰ��İ�ͷ
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
			//���ݰ��İ�β
			else if(RxDate=='@')
			{
				Get_Flag=1;
				RxFlag=2;
			}
		}
		else if(RxFlag==2)
		{
			//���������ݱ������� ��ȥ�м��+
			for(int i=0;i<3;i++)
			{
				data1[i]=DATA_Temp[i];
			}
			for(int j=0;j<3;j++)
			{
				data2[j]=DATA_Temp[j+4];
			}
			
			//����ת������
			DATA1=Make_Date(data1,3);
			DATA2=Make_Date(data2,3);
			
			//״̬λ����
			RxFlag=0;
			Num=0;
			
		}
				
		USART_ClearITPendingBit(USART3, USART_IT_RXNE); //�Ѿ�����������־λ 
	}  
}
