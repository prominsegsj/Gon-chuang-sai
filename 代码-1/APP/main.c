#include "stm32f10x.h"                  // Device header
#include "board.h"
#include "delay.h"
#include "usart.h"
#include "Emm_V5.h"
#include "USART2.h"
#include "USART3.h"
#include "USART4.h"
#include "USART5.h"
#include "sys.h"
#include "MPU6050.h"
#include "inv_mpu.h"
#include "MPUIIC.h"
#include "OLED.h"
#include "PCA9685.h"
#include "Task_Work.h"
#include "Buzzer.h"
#include "AD.h"

void Init(void)
{
	SystemInit();             							 //����ϵͳ��ʼ��
  Beep_Init();              							 //��������ʼ��
	MPU_Init();               							 //MPU6050ģ�������ʼ��       B3 B4
//	MPU6050_EXTI_Init();     								//MPU6050INT�����ⲿ�жϳ�ʼ��  B5
	mpu_dmp_init();           							 //MPU6050ʹ�ÿ�ĳ�ʼ��
	OLED_Init();              							 //OLED��ʼ��   SPIͨѶ        A8 B12 B13 B14 B15
	AD_Init();                							 //AD��ģת��ͨ����ʼ��	
	USART1_Init();            							 //USART1��ʼ�� �������ͨѶ   A9 A10 TX RX
	USART2_Init(9600);        							 //USART2��ʼ�� �ö�������԰� 
	USART3_Init(9600);        							 //USART3��ʼ�� ��ά��ģ��ͨѶ B10 B11	
	UART4_Init(115200);       							 //UART4��ʼ��  ������ͨѶ     A2 A3  TX RX	
  UART5_Init(9600);         							 //UART5��ʼ��  OpenMv����ͨѶ    P4TX P5RX
	Delay_ms(500);	          							 
	initRingBuff();           							 //��������ʾ��ʼ��
	
	runActionGroup(1,1);	
	Delay_ms(500);	          							 //�ϵ�������ʱ��ʼ��		
	OLED_ShowString(1,1,"Init OK");          
	AD_Power();                              //��ʼ�����
	Delay_ms(500);
}

int main(void)
{ 
	Init();
	
	Run();	
	
	while(1)
		{
			
		}
	
}
