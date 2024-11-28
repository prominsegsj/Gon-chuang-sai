/**
  ******************************************************************************
  * @file    	PCA9685.c
  * @author  	MC_Wang
  * @version 	V1.0
  * @date    	2021年3月27日
  * @brief   	STM32通过模拟IIC协议与PCA9685通讯
 @verbatim
 ===============================================================================
                        ##### How to use this driver #####
 ===============================================================================
    [..]
      (#)使用setPWMFreq(u8 freq)函数初始化PCA9685;
			(++)u8 freq:设置舵机的PWM频率；这里使用u8类型(0~255)
			但PCA9685可以设置最大约为1.6KHz的频率，如果要设置更大频率，需要改形参类型和PCA9685_write(unsigned char reg,unsigned char data)
			有兴趣可以自己试试，我这里就用u8类型，舵机50Hz；将freq = 50;即可
	  (#)使用PCA9685_write(unsigned char reg,unsigned char data)向选中的PCA9685的XX寄存器写XX数据
			(++)unsigned char reg：PCA9685寄存器地址
			(++)unsigned char data:向选中的PCA9685的某寄存器写入的数据
	  (#)使用u8 PCA9685_read(unsigned char reg)向选中的PCA9685的XX寄存器读取XX数据
			(++)unsigned char reg：PCA9685寄存器地址
			(++)返回值为PCA9685某寄存器的数据
	  (#)设置PCA9685某通道的占空比，请使用setPWM(u8 num, u16 on, u16 off)
	  (#)将舵机的角度转换成LEDx_OFF的值，请使用calculate_PWM(u8 angle)
 @endverbatim
  *
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 JXUST@WangMingcong</center></h2>
	******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "PCA9685.h"
#include "IIC.h"
#include "delay.h"
#include "math.h"
#include "usart.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/** @defgroup PCA9685_Private_Functions
  * @{
  */
/** @defgroup PCA9685 Initialization and Configuration
 *  @brief   Initialization and Configuration
 *
 @verbatim
 ===============================================================================
                 ##### PCA9685初始化和设置PCA9685的PWM频率 #####
 ===============================================================================

 @endverbatim
  * @{
  */



/**
  * @brief  向选中的PCA9685的XX寄存器写XX数据
  * @note   先选中PCA9685，STM32先通过IIC向PCA9685写PCA9685设备地址,即PCA9685_adrr：0x80
						再通过形参将数据写入到PCA9685寄存器中
  * @param  unsigned char reg
  *            @arg 寄存器地址
  * @param  unsigned char data
  *            @arg 向寄存器写入的内容
  * @date   2021年3月27日
  * @retval None
  */


void PCA9685_Init()
{
    IIC_Init();
    setPWMFreq(50);//舵机50Hz
    PCA9685_write(PCA9685_MODE1,0x0);
}
void PCA9685_write(unsigned char reg,unsigned char data)
{
    IIC_Start();
    IIC_Send_Byte(PCA9685_adrr);
    IIC_Wait_Ack();
    IIC_Send_Byte(reg);
    IIC_Wait_Ack();
    IIC_Send_Byte(data);
    IIC_Wait_Ack();
    IIC_Stop();
}
/**
  * @brief  向选中的PCA9685的XX寄存器读取XX数据
  * @note   先选中PCA9685，STM32先通过IIC向PCA9685写PCA9685设备地址,即PCA9685_adrr：0x80
						再通过形参读取PCA9685寄存器中的数据
  * @param  unsigned char reg
  *            @arg PCA9685寄存器地址
  * @date   2021年3月27日
  * @retval  读取到寄存器中的数据
  */
u8 PCA9685_read(unsigned char reg)
{
    u8 res;
    IIC_Start();
    IIC_Send_Byte(PCA9685_adrr);
    IIC_Wait_Ack();
    IIC_Send_Byte(reg);
    IIC_Wait_Ack();
    IIC_Start();
    IIC_Send_Byte(PCA9685_adrr|0X01);
    IIC_Wait_Ack();
    res=IIC_Read_Byte(0);
    IIC_Stop();
    return res;
}

/**
  * @brief  初始化PCA9685并设置PWM输出频率
  * @note   1.读出PCA9685的MODE1寄存器的值:oldmode == 0x33 = 0011 0011(不复位、使用内部25MHz时钟、内部地址读写完后自动增加、进入sleep、响应0x70通用IIC地址)
						2.不复位PCA9685且进入sleep模式
						3.操作PCA9685的PRE_SCALE寄存器，设置PCA9685的输出PWM频率
						4.退出sleep模式
  * @param  u8 freq
  *            @arg  设置PCA9685的输出PWM频率（0~255）Hz，也就是PPT中的update_rate
  * @date   2021年3月27日
  * @retval None
  */
void setPWMFreq(u8 freq)
{
    u8 prescale,oldmode,newmode;
    double prescaleval;
    prescaleval = 25000000.0/(4096*freq*0.915);
    prescale = (u8)floor(prescaleval+0.5)-1;//floor()：向下取整函数

    oldmode = PCA9685_read(PCA9685_MODE1);
//测试使用：
//    printf("oldmode = %d\r\n",PCA9685_read(PCA9685_MODE1));//oldmode == 0 = 0x00 = 0000 0000
    newmode = (oldmode&0x7F) | 0x10; // 不复位且进入sleep模式  newmode == 16 = 0x10 = 0001 0000
//    printf("newmode = %d\r\n",newmode);
    PCA9685_write(PCA9685_MODE1, newmode); // go to sleep
    PCA9685_write(PCA9685_PRESCALE, prescale); // set the prescaler
    PCA9685_write(PCA9685_MODE1, oldmode);
		Delay_ms(5);
    PCA9685_write(PCA9685_MODE1, oldmode | 0xa1); //1010 0001   写完复位后，D7位自动清零

//    printf("oldmode = %d\r\n",PCA9685_read(PCA9685_MODE1));//  oldmode == 33 = 0x21 = 0010 0001
}


/**
  * @brief  设置各个通道的ON和OFF寄存器，从而达到改变PCA9685输出PWM的占空比
  * @note   因为PCA9685的PWM是12位的，所以传入的参数为u16（16位才可以装下12位的数据）
						在16位数据中，高四位没用，on和off的值在0~4096 慎用
  * @param  u8 num
  *            @arg	通道号 (0~15)
  * @param  u16 on
  *            @arg 计到LEDX_ON时跳变为高电平
  * @param  u16 off
  *            @arg 计数到LEDX_OFF时跳变为低电平
  * @date  	2021年3月27日
  * @retval None
  */
// 传入芯片内的数16位， iic通讯协议一次只能传输8位，前两行将芯片内部对应通道口清零，分两次传输，一次8位
// 计算某个通道地址 例如LED0_ON_L+4*num，num等于0，相当于求芯片手册上6号引脚地址，num等于1，相当于7号引脚地址；
void setPWM(u8 num, u16 on, u16 off)
{
    PCA9685_write(LED0_ON_L+4*num,on);
    PCA9685_write(LED0_ON_H+4*num,on>>8);
    PCA9685_write(LED0_OFF_L+4*num,off);
    PCA9685_write(LED0_OFF_H+4*num,off>>8);
}



/**
  * @brief  将舵机的角度计算成12位LEDX_OFF的值
  * @author MC_Wang
  * @note   通过计算公式，将舵机的角度计算成12位LEDX_OFF的值
计算公式：PWM = 4096*(0.5 + (2.5-0.5)*(u8 angle)/180)/20

  * @param  u8 angle
  *            @arg 舵机角度
  * @date  	2021年3月27日
  * @retval 12位LEDX_OFF的值
  */
u16 calculate_PWM(u8 angle)
{
    return (int)(204.8*(0.5+angle*1.0/90));
}

/************************ (C) COPYRIGHT JXUST@WangMingcong *****END OF FILE****/

/************************ (C) COPYRIGHT JXUST@WangMingcong *****END OF FILE****/
void crazyMe(int i,u16 a,u16 b,u16 tim,u8 xf)//i：控制位 a：目前角度  b：目标角度	tim：每一步的时间		xf：动作分隔次数
{
	u16 k,m;
	u8 s;
	for(s=0;s<=xf;s++)
	{
		if(a>b)
		{
			k=calculate_PWM(a)-calculate_PWM(b);
		}
		else
			k=calculate_PWM(b)-calculate_PWM(a);;
		if(a>b)
		{
			m=calculate_PWM(a)-k*s/xf;
		}
		else
			m=calculate_PWM(a)+k*s/xf;
		setPWM(i,0,m);
		Delay_ms(tim);
	}
}
