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
	SystemInit();             							 //整体系统初始化
  Beep_Init();              							 //蜂鸣器初始化
	MPU_Init();               							 //MPU6050模块基本初始化       B3 B4
//	MPU6050_EXTI_Init();     								//MPU6050INT引脚外部中断初始化  B5
	mpu_dmp_init();           							 //MPU6050使用库的初始化
	OLED_Init();              							 //OLED初始化   SPI通讯        A8 B12 B13 B14 B15
	AD_Init();                							 //AD数模转换通道初始化	
	USART1_Init();            							 //USART1初始化 步进电机通讯   A9 A10 TX RX
	USART2_Init(9600);        							 //USART2初始化 幻尔舵机调试板 
	USART3_Init(9600);        							 //USART3初始化 二维码模块通讯 B10 B11	
	UART4_Init(115200);       							 //UART4初始化  串口屏通讯     A2 A3  TX RX	
  UART5_Init(9600);         							 //UART5初始化  OpenMv串口通讯    P4TX P5RX
	Delay_ms(500);	          							 
	initRingBuff();           							 //串口屏显示初始化
	
	runActionGroup(1,1);	
	Delay_ms(500);	          							 //上电整体延时初始化		
	OLED_ShowString(1,1,"Init OK");          
	AD_Power();                              //初始化完成
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
