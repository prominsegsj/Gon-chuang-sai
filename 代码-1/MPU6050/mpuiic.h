#ifndef __MPUIIC_H
#define __MPUIIC_H
#include "sys.h"

//����ԭ��MPU6050ͨѶ������
	   		   
//IO��������  B7
#define MPU_SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=8<<28;} 
#define MPU_SDA_OUT() {GPIOB->CRL&=0X0FFF0FFF;GPIOB->CRL|=3<<28;}


//IO��������	 
#define MPU_IIC_SCL    PBout(6) 		//SCL 
#define MPU_IIC_SDA    PBout(7) 		//SDA	 
#define MPU_READ_SDA   PBin(7) 		//����SDA 

//IIC���в�������
void MPU_IIC_Delay(void);				//MPU IIC��ʱ����
void MPU_IIC_Init(void);        //��ʼ��IIC��IO��	
void MPU6050_EXTI_Init(void);   //��ʼ��MPU6050�ⲿ�ж�IO��
void MPU_IIC_Start(void);				//����IIC��ʼ�ź�
void MPU_IIC_Stop(void);	  			//����IICֹͣ�ź�
void MPU_IIC_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 MPU_IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
u8 MPU_IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void MPU_IIC_Ack(void);					//IIC����ACK�ź�
void MPU_IIC_NAck(void);				//IIC������ACK�ź�

void IMPU_IC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 MPU_IIC_Read_One_Byte(u8 daddr,u8 addr);	  
#endif
















