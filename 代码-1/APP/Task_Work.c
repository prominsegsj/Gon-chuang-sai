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

//OenMv����
#define See_Ring  1
#define See_Color 2
uint8_t Flag_Color=10;
uint8_t Flag_Ring=10;
extern uint8_t Color_Data[3];
extern uint8_t Ring_Data[2];

//������Ʋ���
#define D_TIM 350   //��Ҫ���ڵ�����Ƶ���ʱ
uint8_t motor_Flag=0;
uint8_t cmd[3]={3,0x3A,0x6B}; //���ͻ�ȡ�����־λ ��ַ ������ У��λ
uint8_t DIR[4]={0}; //���ڴ洢�������
struct Motor{
	uint8_t *P_DIR;
	uint32_t POS;
	uint16_t V;
	uint8_t Acc;	
}MOTOR;

//��ά��ɨ��ģ��
extern int DATA1;
extern int DATA2;
uint8_t Order_Color_1[3]={0};
uint8_t Order_Color_2[3]={0};

//��е�۲���
extern uint8_t Servo_Flag;

//�����˶�����
uint8_t Car_D=2;

void Contral_Car(uint8_t Order,float Pos,uint32_t Angle,uint16_t V,uint8_t Acc)
{
	int a=0;
	MOTOR.P_DIR=DIR;
	MOTOR.V=V;
	MOTOR.Acc=Acc;
		
	//��Ҫ���� ����ٶȷ���
	switch(Order)
	{
		case Front:{MOTOR.P_DIR[0]=0;MOTOR.P_DIR[1]=0;MOTOR.P_DIR[2]=1;MOTOR.P_DIR[3]=1;MOTOR.POS=3200*Pos;}break;//ǰ��
		case Back:{MOTOR.P_DIR[0]=1;MOTOR.P_DIR[1]=1;MOTOR.P_DIR[2]=0;MOTOR.P_DIR[3]=0;MOTOR.POS=3200*Pos;}break; //����
		case Left:{MOTOR.P_DIR[0]=1;MOTOR.P_DIR[1]=0;MOTOR.P_DIR[2]=1;MOTOR.P_DIR[3]=0;MOTOR.POS=3200*Pos;}break; //��ƽ��
		case Right:{MOTOR.P_DIR[0]=0;MOTOR.P_DIR[1]=1;MOTOR.P_DIR[2]=0;MOTOR.P_DIR[3]=1;MOTOR.POS=3200*Pos;}break;//��ƽ��
		case Turn_L:{MOTOR.P_DIR[0]=1;MOTOR.P_DIR[1]=1;MOTOR.P_DIR[2]=1;MOTOR.P_DIR[3]=1;MOTOR.V=300;MOTOR.Acc=110;MOTOR.POS=Angle;}break;//����ת
		case Turn_R:{MOTOR.P_DIR[0]=0;MOTOR.P_DIR[1]=0;MOTOR.P_DIR[2]=0;MOTOR.P_DIR[3]=0;MOTOR.V=300;MOTOR.Acc=110;MOTOR.POS=Angle;}break;	//����ת		
	}
	//Ԥ�������˶���ͻ
	while(motor_Flag)
	{
		// �������� ��ȡ���״̬
		usart_SendCmd(cmd, 3);
		Delay_ms(10);
			
		if((rxCmd[2]&0x02)==0x02)
		{				
			Delay_ms(D_TIM);
			//�����λ
			motor_Flag=0;
		}
		//��ֹ��ʱ����
		if(a>900)
		{
			Delay_ms(D_TIM);
			//�����λ
			motor_Flag=0;				
		}
		a++;
	}
	//�˶�ʵ�ʿ���
	Emm_V5_Pos_Control(1, MOTOR.P_DIR[0], MOTOR.V, MOTOR.Acc, MOTOR.POS, 0, 1);
	Delay_ms(10);
	Emm_V5_Pos_Control(2, MOTOR.P_DIR[1], MOTOR.V, MOTOR.Acc, MOTOR.POS, 0, 1);
	Delay_ms(10);
	Emm_V5_Pos_Control(3, MOTOR.P_DIR[2], MOTOR.V, MOTOR.Acc, MOTOR.POS, 0, 1);
	Delay_ms(10);
	Emm_V5_Pos_Control(4, MOTOR.P_DIR[3], MOTOR.V, MOTOR.Acc, MOTOR.POS, 0, 1);
	Delay_ms(10);
			
	Emm_V5_Synchronous_motion(0);
	Delay_ms(10);
	motor_Flag=1;
		
}

//����С��ת�� ����ת�����
void Contral_Turn_Car(uint8_t Order,uint8_t Angle)
{
	uint32_t ANGLE;
	switch(Order)
	{
		case Turn_L:{ANGLE=44.2*Angle;Contral_Car(Turn_L,0,ANGLE,0,0);}break;
		case Turn_L1:{ANGLE=44.5*Angle;Contral_Car(Turn_L,0,ANGLE,0,0);}break;
		case Turn_L2:{ANGLE=43.7*Angle;Contral_Car(Turn_L,0,ANGLE,0,0);}break;
		case Turn_L3:{ANGLE=44.1*Angle;Contral_Car(Turn_L,0,ANGLE,0,0);}break;
		case Turn_R:{ANGLE=44.9*Angle;Contral_Car(Turn_R,0,ANGLE,0,0);}break;
		case Turn_R1:{ANGLE=44.9*Angle;Contral_Car(Turn_R,0,ANGLE,0,0);}break;
		case Turn_R2:{ANGLE=44.87*Angle;Contral_Car(Turn_R,0,ANGLE,0,0);}break;
		case Turn_R3:{ANGLE=44.83*Angle;Contral_Car(Turn_R,0,ANGLE,0,0);}break;		
	}
}

//��ԭ��������ץȡ
void Task_Catch_Work(uint8_t *Color_Order)
{
	//���������λ��
	if(Color_Data[2]<20)
	{
		Contral_Car(Left,0.07,0,380,100);		
	}
	else if(Color_Data[2]>68)
	{
		Contral_Car(Right,0.2,0,380,100);
	}
	else if(Color_Data[2]>59&&Color_Data[2]<=68)
	{
		Contral_Car(Right,0.1,0,380,100);		
	}
	for(int a=0;a<3;a++)
	{
		while(1)
		{
			if(Color_Data[0]==Color_Order[a])
			{
				runActionGroup(3+a,1);
				runActionGroup(2,1);						
				break;
			}
			else 
			{
				OpenMv_Part(See_Color);
			}											
		}
	}			
}

//�˺�����һ���Է��������
void Put(uint8_t *Color_Order)
{
	if(Ring_Data[1]<=65&&Ring_Data[1]>=61)
	{
		Contral_Car(Left,0.125,0,380,170);						
	}
	else
	{
		Contral_Car(Left,0.1,0,380,170);		
	}	
	for(int i=0;i<3;i++)
	{
		runActionGroup(7+i,1);
		runActionGroup(9+Color_Order[i],1);					
	}		
}

//�˺�����Ҫ���ڷ����������
void Car_Adjust(uint8_t Want_Postion)
{
	int Temp=0;
	if(Car_D==Want_Postion)
	{
		if(Ring_Data[1]<=65&&Ring_Data[1]>=61)
		{
			Contral_Car(Left,0.125,0,380,170);						
		}
		else
		{
			Contral_Car(Left,0.1,0,380,170);		
		}	
	}
	else
	{
		Temp=Car_D-Want_Postion;
		if(Temp>0)
		{
			Contral_Car(Back,0.592*Temp,0,380,100);	
			Ring_Adjust();
			if(Ring_Data[1]<=65&&Ring_Data[1]>=61)
			{
				Contral_Car(Left,0.125,0,380,170);						
			}
			else
			{
				Contral_Car(Left,0.1,0,380,170);						
			}
		}
		else if(Temp<0)
		{
			Temp=-Temp;
			Contral_Car(Front,0.592*Temp,0,380,100);
			Ring_Adjust();
			if(Ring_Data[1]<=65&&Ring_Data[1]>=61)
			{
				Contral_Car(Left,0.125,0,380,170);						
			}
			else 
			{
				Contral_Car(Left,0.1,0,380,170);						
			}
		}
	}
	Car_D=Want_Postion;
}

void LAST_Car_Adjust(uint8_t Want_Postion)
{
	int Temp=0;
	if(Car_D==Want_Postion)
	{
		Car_D=Want_Postion;	
	}
	else
	{
		Temp=Car_D-Want_Postion;
		if(Temp>0)
		{
			Contral_Car(Back,0.592*Temp,0,380,100);	
			Ring_Adjust();
		}
		else if(Temp<0)
		{
			Temp=-Temp;
			Contral_Car(Front,0.592*Temp,0,380,100);
			Ring_Adjust();
		}
	}
	Car_D=Want_Postion;
}


//�˺���Ϊ�ֱ��������������߼�
uint8_t PUT(uint8_t *Color_Order)
{	
	for(int i=0;i<3;i++)
	{
	//�ߵ���Ӧ��Բ��֮ǰ
		Car_Adjust(Color_Order[i]);
	//�����
		runActionGroup(7+i,1);
		runActionGroup(11,1);
		runActionGroup(6,1);
		Contral_Car(Right,0.17,0,380,170);							
	}
	Car_Adjust(2);
}

uint8_t LAST_PUT(uint8_t *Color_Order)
{	
	for(int i=0;i<3;i++)
	{
	//�ߵ���Ӧ��Բ��֮ǰ
		LAST_Car_Adjust(Color_Order[i]);
	//�����
		runActionGroup(7+i,1);
		runActionGroup(19,1);
		runActionGroup(6,1);
	}
	Contral_Car(Left,0.037,0,380,170);								
}

//�˺���Ϊһ����ץ����
void Catch(uint8_t *Color_Order)
{
	for(int i=0;i<3;i++) //13-15
	{
		runActionGroup(12+Color_Order[i],1);					
		runActionGroup(16+i,1);
		runActionGroup(2,1);
	}		
}

//����Բ��ʶ������������λ�ôﵽ�������λ��
void Ring_Adjust(void)
{
	uint8_t CAR_FLAG=1;
	float Temp1=0;
	float Temp2=0;

	while(CAR_FLAG)
	{
		OpenMv_Part(See_Ring);
		if(Ring_Data[1]==6)//��ʾû�гɹ�ʶ��Բ��
		{
			Contral_Car(Front,0.1,0,200,80);
			Contral_Car(Right,0.09,0,200,80);																					
			continue;	
		}
		//����ɹ�ʶ��Բ�� �����������Щ�жϲ���
		if((Ring_Data[0]<=82&&Ring_Data[0]>=68)&&(Ring_Data[1]<=73&&Ring_Data[1]>=61))
		{
			CAR_FLAG=0;
			break;
		}
		else
		{
			//����x���λ��
			if(Ring_Data[0]<68&&Ring_Data[0]>20)
			{
				Temp1=0.3066-0.0041*Ring_Data[0];
				Contral_Car(Back,Temp1,0,200,100);
			}
			else if(Ring_Data[0]>82||Ring_Data[0]<=20)
			{
				Contral_Car(Front,0.1,0,200,100);				
			}
			//����y���λ��
			if(Ring_Data[1]<61&&Ring_Data[1]>45)
			{				
				if(Ring_Data[1]<61&&Ring_Data[1]>=53)//Ԥ������ֵ��С���⿨��
				{
					Contral_Car(Left,0.02,0,200,100);														
				}
				else if(Ring_Data[1]<53&&Ring_Data[1]>45)
				{
					Temp2=0.364-0.0052*Ring_Data[1];
					Contral_Car(Left,Temp2,0,200,100);									
				}
			}
			else if(Ring_Data[1]<95&&Ring_Data[1]>73)
			{
				if(Ring_Data[1]>73&&Ring_Data[1]<80)//Ԥ������ֵ��С���⿨��
				{
					Contral_Car(Right,0.04,0,200,100);																		
				}
				else if(Ring_Data[1]<95&&Ring_Data[1]>=80)
				{
					Temp2=0.0042*Ring_Data[1]-0.299;
					Contral_Car(Right,Temp2,0,200,100);													
				}
			}
		}
	}	  
}

//OpenMvʶ�𲿷�
uint8_t OpenMv_Part(uint8_t Command)
{
	switch(Command)
	{
		case See_Color:
		{
			Flag_Color=0;
			USART_SendData(UART5,0x02);
			while(Flag_Color!=1);
		}break;
		case See_Ring:
		{
			Flag_Ring=0;
			USART_SendData(UART5,0x01);
			while(Flag_Ring!=1);
		}break;				
	}
}

//���ڵ�ѹ����
void AD_Power(void)
{
	uint16_t ADValue;			
	
		ADValue = AD_GetValue();					
		
		if(ADValue>4000)
		{
			OLED_ShowString(4, 1, "Power: Normal");       
		}
		else
		{
			OLED_ShowString(4, 1, "Power: Danger");       			
		}	
}

////////////////////////////////////�˴�Ϊһ���ֽ��ߣ����沿��Ϊ���Բ���/////////////////////////////////////////////////////////////////////////////////
float pitch,roll,yaw; 			//�����ǡ�����ǡ������					  		

extern uint8_t Flag;

extern uint8_t abc;

extern int Get_Flag;

//void Scan_MpuVal(void)
//{
//		if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0)			//dmp����õ�����
//			{ 	
//				if(yaw_count==120000)yaw_count=0;				
//				myaw=yaw;
//				myaw=myaw-0.0001*yaw_count-0.49;				
//			}	
//}

//������Ҫ��ģ�鿪���Ĳ��Բ���
void Try(uint8_t num)
{
	switch(num)
	{
		case 1://�������
		{
			uint8_t t=1;

//      //�ϵ���ʱ2��ȴ�Emm_V5.0�ջ���ʼ�����
//			Delay_ms(2000);
//       Beep_ON();
      //λ��ģʽ������CW���ٶ�1000RPM�����ٶ�0����ʹ�üӼ���ֱ����������������3200��16ϸ���·���3200��������תһȦ��������˶�
			Emm_V5_Pos_Control(1, 0, 300, 100, 3200*t, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(2, 0, 300, 100, 3200*t, 0, 1);
			Delay_ms(10);
			
			Emm_V5_Pos_Control(3, 1, 300, 100, 3200*t, 0, 1);
			Delay_ms(10);
			
			Emm_V5_Pos_Control(4, 1, 300, 100, 3200*t, 0, 1);
			Delay_ms(10);
			
			Emm_V5_Synchronous_motion(0);
			Delay_ms(10);
			
      //�����λ�ź� = 0x03 & 0x02 = �����λ;
//			Emm_V5_Pos_Control(5, 0, 300, 100, 800, 0, 0);
//			Delay_ms(10);

//			Emm_V5_Pos_Control(6, 0, 300, 100, 200, 0, 0);
//			Delay_ms(10);

//      //�ȴ���������������ݻ���������rxCmd�ϣ�����ΪrxCount
//			while(rxFrameFlag == false);
//			rxFrameFlag = false;
//			Delay_ms(500);	
//        uint8_t ab;
				  uint8_t cmd[16] = {0};  
					// װ������
					cmd[0] =  1;                       // ��ַ
					cmd[1] =  0x3A;                      
					cmd[2] =  0x6B;                    
//       Beep_ON();
			while(1)
			{					
					// ��������
					usart_SendCmd(cmd, 3);
				  Delay_ms(500);
				  if(rxFrameFlag==true)
					{
						 
						if((rxCmd[2]&0x02)==0x02)
						{
							  //�����λ
							USART_SendData(USART2,12);
							Beep_ON();
						}
						 Serial_SendArray_USART2(rxCmd,4);
//						 USART_SendData(USART2,ab);
					   rxFrameFlag=false;
					}
			}			
		}break;
		
		case 2://��ʾ������ ��ά�����
		{
			while(Get_Flag==0)
			{
			  Set_ON();//������ά��ɨ��ģ��	
				Delay_ms(10);
			}
			int a=DATA1;
			int b=DATA2;
			Delay_ms(100);
			
			    TJCPrintf("n0.val=%d",000);

		    	TJCPrintf("n1.val=%d",DATA1);

			    TJCPrintf("n2.val=%d",DATA2);
			    Delay_ms(100);									

			 for(int i=3;i>0;i--)
			{
				Order_Color_1[i-1]=a%10;
				a=a/10;
				
				Order_Color_2[i-1]=b%10;
				b=b/10;
			}
			
		}break;
		//OLED��ʾ����
		case 3:
		{
       OLED_ShowNum(1,1,222,3);
		}break;
		case 4://�������
		{
			Delay_ms(5000);
			while(1)
			{
	      mpu_dmp_get_data(&pitch,&roll,&yaw);
				OLED_ShowNum(2,1,yaw,3);
				OLED_ShowNum(2,5,pitch,3);
				OLED_ShowNum(2,9,roll,3);
				
				Delay_ms(100);
			}
		}break;
		case 5://PCA9685����
		{
			Beep_ON();
		  setPWM(2,0,calculate_PWM(85));	
				Delay_ms(1000);
//			setPWM(2,0,calculate_PWM(88));
//				Delay_ms(1000);
//			setPWM(2,0,calculate_PWM(55));
//				Delay_ms(1000);
//			setPWM(2,0,calculate_PWM(88));
			
		}break;
		case 6://�����ܷ��߼�����
		{
					
		}break;
		case 7://������ģ���ʼ��
		{
			Emm_V5_Pos_Control(5, 1, 300, 100, 3200*0.5, 0, 0); 
			Delay_ms(10);	
			
      //λ��ģʽ������CW���ٶ�1000RPM�����ٶ�0����ʹ�üӼ���ֱ����������������3200��16ϸ���·���3200��������תһȦ��������˶�
			Emm_V5_Pos_Control(6, 1, 300, 100, 3200*1.4, 0, 0); //�½�
			Delay_ms(10);
			
			setPWM(2,0,calculate_PWM(140));	
			Delay_ms(10);			
		}break;
		case 8://��ѹ����
		{
			while (1)
			{
//				ADValue = AD_GetValue();					
////				Voltage = (float)ADValue / 4095 * 3.3;	
//				
//				OLED_ShowNum(2, 9, ADValue, 4);				
////				OLED_ShowNum(2, 9, Voltage, 1);				
////				OLED_ShowNum(2, 11, (uint16_t)(Voltage * 100) % 100, 2);	
//				
//				Delay_ms(100);			
			}

		}break;
		case 9://��ѹ����
		{
			runActionGroup(6,1);
			Delay_ms(1000);
			while (1)
			{
				OpenMv_Part(See_Ring);
				Delay_ms(10);
			}

		}break;
		
	}
}

//����Ϊ���������߼��ĵ��Բ���
void Run()
{
		Contral_Car(Left,0.95,0,100,130); 
		Delay_ms(2000);
		Contral_Car(Front,3.15,0,100,130); 	
		Try(2);
		runActionGroup(2,1);
		Contral_Car(Front,2.842,0,100,130);
		Contral_Car(Right,0.43,0,100,130);
		OpenMv_Part(See_Color);		
		Task_Catch_Work(Order_Color_1);

		Contral_Car(Left,0.4,0,100,130);
		Contral_Car(Back,1.7,0,100,130);
		Contral_Turn_Car(Turn_L,90);
		Contral_Car(Front,7.1,0,100,70);
		Contral_Turn_Car(Turn_L1,90);
		Contral_Car(Right,0.1,0,450,170);
	//�ִ�ּӹ���
		runActionGroup(6,1);	
		Delay_ms(1000);
		Ring_Adjust();
		//ץ7-9��˳��ץȡ  ��10-12��123
		PUT(Order_Color_1);

		runActionGroup(2,1);
		
		//ץ13-15��˳��ץȡ ��16-18		
		Catch(Order_Color_1);

			Contral_Car(Left,0.3,0,100,130);
			Contral_Car(Back,3.5,0,100,130);
		Contral_Turn_Car(Turn_R,90);
			Contral_Car(Back,3.28,0,100,130);
			Contral_Car(Right,0.15,0,100,130);
			runActionGroup(6,1);	
			Delay_ms(1000);
			Ring_Adjust();
		//ץ7-9��˳��ץȡ  ��10-12��123
		PUT(Order_Color_1);

		runActionGroup(2,1);
		
			Contral_Car(Left,0.2,0,100,130);
			Contral_Car(Back,3.65,0,100,10);
		Contral_Turn_Car(Turn_R1,90);
			Contral_Car(Back,1.86,0,100,100);
			Contral_Car(Right,0.2,0,100,100);
			OpenMv_Part(See_Color);		
			Task_Catch_Work(Order_Color_2);
		
/***************************�ڶ���*******************************************/

		Contral_Car(Left,0.3,0,100,100);
		Contral_Car(Back,1.65,0,100,100);
		Contral_Turn_Car(Turn_L2,90);
		Contral_Car(Front,7.34,0,100,70);
		Contral_Turn_Car(Turn_L3,90);
		runActionGroup(6,1);	
		Delay_ms(1000);
		Ring_Adjust();
		//ץ7-9��˳��ץȡ  ��10-12��123
		PUT(Order_Color_2);

		runActionGroup(2,1);
		
		//ץ13-15��˳��ץȡ ��16-18		
		Catch(Order_Color_2);
		
			Contral_Car(Left,0.4,0,100,100);
			Contral_Car(Back,3.5,0,100,100);
		Contral_Turn_Car(Turn_R2,90);
			Contral_Car(Back,3.3,0,100,100);
			Contral_Car(Right,0.15,0,100,100);
			runActionGroup(6,1);	
			Delay_ms(1000);
			Ring_Adjust();
//		//ץ7-9��˳��ץȡ  ��10-12��123
			LAST_PUT(Order_Color_2);
					
			Contral_Car(Left,0.3,0,100,100);
			Contral_Car(Back,3.6+Car_D*0.15,0,250,150);
			Contral_Turn_Car(Turn_R3,90);
			Contral_Car(Back,7.6,0,250,150);
			Contral_Car(Right,0.6,0,100,100);
		
}
