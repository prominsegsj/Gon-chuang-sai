#include "stm32f10x.h"                  // Device header
#include "USART4.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>

#define STR_LENGTH 100

typedef struct
{
	uint16_t Head;
	uint16_t Tail;
	uint16_t Lenght;
	uint8_t  Ring_data[RINGBUFF_LEN];
}RingBuff_t;

RingBuff_t ringBuff;	//����һ��ringBuff�Ļ�����

/********************************************************
��������  	UART4_Init
���ߣ�    	
���ڣ�    	2024.08.18
���ܣ�    	��ʼ��uart4,���ڷ������ݸ�������
���������	
����ֵ�� 		
�޸ļ�¼��
**********************************************************/
void UART4_Init(uint32_t __Baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOC,ENABLE);	
	
	
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin           = GPIO_Pin_10;//TX����
	GPIO_InitStructure.GPIO_Mode          = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed         = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin           = GPIO_Pin_11;//RX����
	GPIO_InitStructure.GPIO_Mode          = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
  
	USART_InitTypeDef USART_InitStructure;//�������ô��ڵĽṹ�����

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);//��������4��ʱ��

	USART_DeInit(UART4);//�����˼�ǽ���˴��ڵ���������

	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate              = __Baud;//���ò�����
	USART_InitStructure.USART_WordLength            = USART_WordLength_8b;//�ֽڳ���Ϊ8bit
	USART_InitStructure.USART_StopBits              = USART_StopBits_1;//1��ֹͣλ
	USART_InitStructure.USART_Parity                = USART_Parity_No ;//û��У��λ
	USART_InitStructure.USART_Mode                  = USART_Mode_Rx | USART_Mode_Tx;//����������Ϊ�շ�ģʽ
	USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None; //���ṩ���� 
	USART_Init(UART4,&USART_InitStructure);//����ز�����ʼ��������1
	
	USART_ClearFlag(UART4,USART_FLAG_RXNE);//��ʼ����ʱ���������λ
	USART_ClearFlag(UART4,USART_FLAG_IDLE);//��ʼ����ʱ���������λ

	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//��ʼ���ý����ж�
	//USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);//��ʼ���ÿ����ж�

	USART_Cmd(UART4,ENABLE);//��������1
	
	NVIC_InitTypeDef NVIC_InitStructure;//�жϿ��ƽṹ���������

	NVIC_InitStructure.NVIC_IRQChannel                    = 52;//�ж�ͨ��ָ��ΪUSART4
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 1;//�����ȼ�Ϊ1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 2;//�����ȼ�Ϊ1
	NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;//ȷ��ʹ��
	NVIC_Init(&NVIC_InitStructure);//��ʼ�����ô��ж�ͨ��
		
}

/********************************************************
��������  	TJC_SendData
���ߣ�    	
���ڣ�    	2024.08.18
���ܣ�    	�򴮿ڴ�ӡ����,��Ҫ�ƶ����͵��ַ�������
���������	
����ֵ�� 		
�޸ļ�¼��
**********************************************************/
void TJC_SendData(uint8_t *data, uint16_t len)
{
	while(len--)
	{
		//�ȴ��������ݼĴ���Ϊ��
		
		USART_SendData(UART4, *data++);
		while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
	}

}

/********************************************************
��������  	TJCPrintf
���ߣ�    	wwd
���ڣ�    	2022.10.08
���ܣ�    	�򴮿ڴ�ӡ����,ʹ��ǰ�뽫USART_SCREEN_write���������Ϊ��ĵ�Ƭ���Ĵ��ڷ��͵��ֽں���
���������	0�ο�printf
����ֵ�� 		��ӡ�����ڵ�����
�޸ļ�¼��
**********************************************************/

void TJCPrintf(const char *str, ...)
{
	uint8_t end = 0xff;
	char buffer[STR_LENGTH+1];  // ���ݳ���
	va_list arg_ptr;
	va_start(arg_ptr, str);
	int len = vsnprintf(buffer, STR_LENGTH+1, str, arg_ptr);
	va_end(arg_ptr);
	for(int i = 0; i < len; i++)
	{
		USART_SendData(UART4, buffer[i]);
		while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);	//�ȴ��������
	}

	USART_SendData(UART4, end);			//���������Ϊ��ĵ�Ƭ���Ĵ��ڷ��͵��ֽں���
	while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);	//�ȴ��������
	USART_SendData(UART4, end);			//���������Ϊ��ĵ�Ƭ���Ĵ��ڷ��͵��ֽں���
	while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);	//�ȴ��������
	USART_SendData(UART4, end);			//���������Ϊ��ĵ�Ƭ���Ĵ��ڷ��͵��ֽں���
	while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);	//�ȴ��������
}


/********************************************************
��������  	initRingBuff
���ߣ�
���ڣ�    	2022.10.08
���ܣ�    	��ʼ�����λ�����
���������
����ֵ�� 		void
�޸ļ�¼��
**********************************************************/
void initRingBuff(void)
{
  //��ʼ�������Ϣ
  ringBuff.Head = 0;
  ringBuff.Tail = 0;
  ringBuff.Lenght = 0;
}


/********************************************************
��������  	writeRingBuff
���ߣ�
���ڣ�    	2022.10.08
���ܣ�    	�����λ�����д������
���������
����ֵ�� 		void
�޸ļ�¼��
**********************************************************/
void writeRingBuff(uint8_t data)
{
  if(ringBuff.Lenght >= RINGBUFF_LEN) //�жϻ������Ƿ�����
  {
    return ;
  }
  ringBuff.Ring_data[ringBuff.Tail]=data;
  ringBuff.Tail = (ringBuff.Tail+1)%RINGBUFF_LEN;//��ֹԽ��Ƿ�����
  ringBuff.Lenght++;

}

/********************************************************
��������  	deleteRingBuff
���ߣ�
���ڣ�    	2022.10.08
���ܣ�    	ɾ�����ڻ���������Ӧ���ȵ�����
���������		Ҫɾ���ĳ���
����ֵ�� 		void
�޸ļ�¼��
**********************************************************/
void deleteRingBuff(uint16_t size)
{
	if(size >= ringBuff.Lenght)
	{
	    initRingBuff();
	    return;
	}
	for(int i = 0; i < size; i++)
	{

		if(ringBuff.Lenght == 0)//�жϷǿ�
		{
		initRingBuff();
		return;
		}
		ringBuff.Head = (ringBuff.Head+1)%RINGBUFF_LEN;//��ֹԽ��Ƿ�����
		ringBuff.Lenght--;

	}

}

/********************************************************
��������  	read1BFromRingBuff
���ߣ�
���ڣ�    	2022.10.08
���ܣ�    	�Ӵ��ڻ�������ȡ1�ֽ�����
���������		position:��ȡ��λ��
����ֵ�� 		����λ�õ�����(1�ֽ�)
�޸ļ�¼��
**********************************************************/
uint8_t read1BFromRingBuff(uint16_t position)
{
	uint16_t realPosition = (ringBuff.Head + position) % RINGBUFF_LEN;

	return ringBuff.Ring_data[realPosition];
}

/********************************************************
��������  	getRingBuffLenght
���ߣ�
���ڣ�    	2022.10.08
���ܣ�    	��ȡ���ڻ���������������
���������
����ֵ�� 		���ڻ���������������
�޸ļ�¼��
**********************************************************/
uint16_t getRingBuffLenght()
{
	return ringBuff.Lenght;
}

/********************************************************
��������  	isRingBuffOverflow
���ߣ�
���ڣ�    	2022.10.08
���ܣ�    	�жϻ��λ������Ƿ�����
���������
����ֵ�� 		1:���λ��������� , 2:���λ�����δ��
�޸ļ�¼��
**********************************************************/
uint8_t isRingBuffOverflow()
{
	return ringBuff.Lenght == RINGBUFF_LEN;
}


#if !defined(__MICROLIB)
//��ʹ��΢��Ļ�����Ҫ�������ĺ���
#if (__ARMCLIB_VERSION <= 6000000)
//�����������AC5  �Ͷ�����������ṹ��
struct __FILE
{
	int handle;
};
#endif

FILE __stdout;

//����_sys_exit()�Ա���ʹ�ð�����ģʽ
void _sys_exit(int x)
{
	x = x;
}
#endif

/******** ����4 �жϷ����� ***********/
void UART4_IRQHandler(void)
{
	if(USART_GetITStatus(UART4, USART_IT_RXNE) == SET)//�ж��ǲ���������жϷ���
	{
//		USART_SendData(UART4,USART_ReceiveData(UART4));//�ֽ����ݷ���ȥ(������֤)

		writeRingBuff(USART_ReceiveData(UART4));
		USART_ClearITPendingBit(UART4, USART_IT_RXNE); //�Ѿ�����������־λ 
	}  
}
