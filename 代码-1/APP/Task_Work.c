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

//OenMv部分
#define See_Ring  1
#define See_Color 2
uint8_t Flag_Color=10;
uint8_t Flag_Ring=10;
extern uint8_t Color_Data[3];
extern uint8_t Ring_Data[2];

//电机控制部分
#define D_TIM 500   //主要用于电机控制的延时
uint8_t motor_Flag=0;

//二维码扫码模块
extern int DATA1;
extern int DATA2;
uint8_t Order_Color_1[3]={0};
uint8_t Order_Color_2[3]={0};

//机械臂部分
extern uint8_t Servo_Flag;

//任务运动部分
uint8_t Car_D=2;

void Set_Go(uint8_t Order,float Pos,uint8_t Angle,uint16_t V,uint8_t Acc)
{

	uint32_t Angle_Num; 
	int a=0;
	uint8_t cmd[3] = {0};  
	// 装载命令
	cmd[0] =  3;                       // 地址
	cmd[1] =  0x3A;                    //功能码  
	cmd[2] =  0x6B;                    //效验位
	
	float t=Pos;
	
	//前进	
	if(Order==Front)
	{
		while(motor_Flag)
		{
			// 发送命令 获取电机状态
			usart_SendCmd(cmd, 3);
			Delay_ms(10);
			
			if((rxCmd[2]&0x02)==0x02)
			{				
				Delay_ms(D_TIM);
				//电机到位
				motor_Flag=0;
			}
			//防止超时卡死
			if(a>900)
			{
				Delay_ms(D_TIM);
				//电机到位
				motor_Flag=0;				
			}
			a++;
		}		

			Emm_V5_Pos_Control(1, 0, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(2, 0, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(3, 1, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(4, 1, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			
			Emm_V5_Synchronous_motion(0);
			Delay_ms(10);
		  motor_Flag=1;
		
	}
	
	//后退
	else if(Order==Back)
	{
		while(motor_Flag)
		{
			// 发送命令 获取电机状态
			usart_SendCmd(cmd, 3);
			Delay_ms(10);
			
			if((rxCmd[2]&0x02)==0x02)
			{				
				Delay_ms(D_TIM);
				//电机到位
				motor_Flag=0;
			}
			//防止超时卡死
			if(a>900)
			{
				Delay_ms(D_TIM);
				//电机到位
				motor_Flag=0;				
			}
			a++;
		}		
		
			Emm_V5_Pos_Control(1, 1, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(2, 1, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(3, 0, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(4, 0, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			
			Emm_V5_Synchronous_motion(0);
			Delay_ms(10);
		  motor_Flag=1;
		
	}
	
	//左移
	else if(Order==Left)
	{
		while(motor_Flag)
		{
			// 发送命令 获取电机状态
			usart_SendCmd(cmd, 3);
			Delay_ms(10);
			
			if((rxCmd[2]&0x02)==0x02)
			{				
				Delay_ms(D_TIM);
				//电机到位
				motor_Flag=0;
			}
			//防止超时卡死
			if(a>900)
			{
				Delay_ms(D_TIM);
				//电机到位
				motor_Flag=0;				
			}
			a++;
		}	
		
			Emm_V5_Pos_Control(1, 1, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(2, 0, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(3, 1, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(4, 0, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			
			Emm_V5_Synchronous_motion(0);
			Delay_ms(10);
		  motor_Flag=1;		
	}	
	
	//右移
	else if(Order==Right)
	{
		while(motor_Flag)
		{
			// 发送命令 获取电机状态
			usart_SendCmd(cmd, 3);
			Delay_ms(10);
			
			if((rxCmd[2]&0x02)==0x02)
			{				
				Delay_ms(D_TIM);
				//电机到位
				motor_Flag=0;
			}
			//防止超时卡死
			if(a>900)
			{
				Delay_ms(D_TIM);
				//电机到位
				motor_Flag=0;				
			}
			a++;
		}		
		
			Emm_V5_Pos_Control(1, 0, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(2, 1, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(3, 0, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(4, 1, V, Acc, 3200*t, 0, 1);
			Delay_ms(10);
			
			Emm_V5_Synchronous_motion(0);
			Delay_ms(10);
		  motor_Flag=1;		
	}	
	
	//转向
	else if(Order==Turn_L)
	{
		while(motor_Flag)
		{
			// 发送命令 获取电机状态
			usart_SendCmd(cmd, 3);
			Delay_ms(10);
			
			if((rxCmd[2]&0x02)==0x02)
			{				
				Delay_ms(D_TIM);
				//电机到位
				motor_Flag=0;
			}
			//防止超时卡死
			if(a>900)
			{
				Delay_ms(D_TIM);
				//电机到位
				motor_Flag=0;				
			}
			a++;
		}		
		
		  Angle_Num=43.84*Angle;
			Emm_V5_Pos_Control(1, 1, 300, 110, Angle_Num, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(2, 1, 300, 110, Angle_Num, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(3, 1, 300, 110, Angle_Num, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(4, 1, 300, 110, Angle_Num, 0, 1);
			Delay_ms(10);
			
			Emm_V5_Synchronous_motion(0);
			Delay_ms(10);
		  motor_Flag=1;
		
	}
	else if(Order==Turn_R)
	{
		while(motor_Flag)
		{
			// 发送命令 获取电机状态
			usart_SendCmd(cmd, 3);
			Delay_ms(10);
			
			if((rxCmd[2]&0x02)==0x02)
			{				
				Delay_ms(D_TIM);
				//电机到位
				motor_Flag=0;
			}
			//防止超时卡死
			if(a>900)
			{
				Delay_ms(D_TIM);
				//电机到位
				motor_Flag=0;				
			}
			a++;
		}		
		  Angle_Num=43.6*Angle;
			Emm_V5_Pos_Control(1, 0, 300, 110, Angle_Num, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(2, 0, 300, 110, Angle_Num, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(3, 0, 300, 110, Angle_Num, 0, 1);
			Delay_ms(10);
			Emm_V5_Pos_Control(4, 0, 300, 110, Angle_Num, 0, 1);
			Delay_ms(10);
			
			Emm_V5_Synchronous_motion(0);
			Delay_ms(10);
		  motor_Flag=1;		
	}
	
}

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

//在原料区物料抓取
void Task_Catch_Work(uint8_t Command)
{
	//调整车身的位置
	if(Color_Data[2]>68)
	{
		Set_Go(Right,0.2,0,380,100);
	}
	else if(Color_Data[2]>59&&Color_Data[2]<=68)
	{
		Set_Go(Right,0.1,0,380,100);		
	}
	switch(Command)
	{
		//第一遍抓取
		case 1:
		{
			for(int a=0;a<3;a++)
			{
				while(1)
				{
					if(Color_Data[0]==Order_Color_1[a])
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
		}break;
		//第二遍抓取
		case 2:
		{
			for(int a=0;a<3;a++)
			{
				while(1)
				{
					if(Color_Data[0]==Order_Color_2[a])
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
		}break;		
	}
}

void Put(uint8_t Command)
{
	if(Command==1)
	{
		for(int i=0;i<3;i++)
		{
			runActionGroup(7+i,1);
			runActionGroup(9+Order_Color_1[i],1);					
		}		
	}
	else if(Command==2)
	{
		for(int i=0;i<3;i++)
		{
			runActionGroup(7+i,1);
			runActionGroup(9+Order_Color_2[i],1);					
		}			
	}
}

void Car_Adjust(uint8_t Want_Postion)
{
	int Temp=0;
	if(Car_D==Want_Postion)
	{
		Set_Go(Left,0.14,0,380,170);		
	}
	else
	{
		Temp=Car_D-Want_Postion;
		if(Temp>0)
		{
			Set_Go(Back,0.55*Temp,0,380,100);	
			Ring_Adjust();
			Set_Go(Left,0.14,0,380,170);					
		}
		else if(Temp<0)
		{
			Temp=-Temp;
			Set_Go(Front,0.55*Temp,0,380,100);
			Ring_Adjust();
			Set_Go(Left,0.14,0,380,170);								
		}
	}
	Car_D=Want_Postion;
}

uint8_t PUT(uint8_t *Color_Order)
{	
	for(int i=0;i<3;i++)
	{
	//走到对应的圆环之前
		Car_Adjust(Color_Order[i]);
	//放物块
		runActionGroup(7+i,1);
		runActionGroup(11,1);
		runActionGroup(6,1);
		Set_Go(Right,0.1,0,380,170);							
	}
	Car_Adjust(2);	
}

void Catch(uint8_t *Color_Order)
{
	for(int i=0;i<3;i++) //13-15
	{
		runActionGroup(12+Color_Order[i],1);					
		runActionGroup(16+i,1);
		runActionGroup(2,1);
	}		
}

//根据圆环识别来调整车身位置达到放置物块位置
void Ring_Adjust(void)
{
	uint8_t CAR_FLAG=1;
	float Temp1=0;
	float Temp2=0;

	while(CAR_FLAG)
	{
		OpenMv_Part(See_Ring);
		if(Ring_Data[1]==6)//表示没有成功识别到圆环
		{
			Set_Go(Front,0.15,0,200,80);
			Set_Go(Right,0.09,0,200,80);																					
			continue;	
		}
		//如果成功识别到圆环 则进行下面这些判断操作
		if((Ring_Data[0]<=82&&Ring_Data[0]>=68)&&(Ring_Data[1]<=73&&Ring_Data[1]>=61))
		{
			CAR_FLAG=0;
			break;
		}
		else
		{
			//调整x轴的位置
			if(Ring_Data[0]<68&&Ring_Data[0]>20)
			{
				Temp1=0.3066-0.0041*Ring_Data[0];
				Set_Go(Back,Temp1,0,200,100);
			}
			else if(Ring_Data[0]>82||Ring_Data[0]<=20)
			{
				Set_Go(Front,0.1,0,200,100);				
			}
			//调整y轴的位置
			if(Ring_Data[1]<61&&Ring_Data[1]>45)
			{				
				if(Ring_Data[1]<61&&Ring_Data[1]>=53)//预防调整值过小避免卡死
				{
					Set_Go(Left,0.02,0,200,100);														
				}
				else if(Ring_Data[1]<53&&Ring_Data[1]>45)
				{
					Temp2=0.364-0.0052*Ring_Data[1];
					Set_Go(Left,Temp2,0,200,100);									
				}
			}
			else if(Ring_Data[1]<95&&Ring_Data[1]>73)
			{
				if(Ring_Data[1]>73&&Ring_Data[1]<80)//预防调整值过小避免卡死
				{
					Set_Go(Right,0.04,0,200,100);																		
				}
				else if(Ring_Data[1]<95&&Ring_Data[1]>=80)
				{
					Temp2=0.0042*Ring_Data[1]-0.299;
					Set_Go(Right,Temp2,0,200,100);													
				}
			}
		}
	}	  
}



void AD_Power(void)
{
	uint16_t ADValue;			//AD实际值
	
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


////////////////////////////////////此处为一条分界线，下面部分为调试部分/////////////////////////////////////////////////////////////////////////////////
float pitch,roll,yaw; 			//俯仰角、横滚角、航向角					  		

extern uint8_t Flag;

extern uint8_t abc;

extern int Get_Flag;

//void Scan_MpuVal(void)
//{
//		if(mpu_dmp_get_data(&pitch,&roll,&yaw)==0)			//dmp处理得到数据
//			{ 	
//				if(yaw_count==120000)yaw_count=0;				
//				myaw=yaw;
//				myaw=myaw-0.0001*yaw_count-0.49;				
//			}	
//}


void Try(uint8_t num)
{
	switch(num)
	{
		case 1://电机测试
		{
			uint8_t t=1;

//      //上电延时2秒等待Emm_V5.0闭环初始化完毕
//			Delay_ms(2000);
//       Beep_ON();
      //位置模式：方向CW，速度1000RPM，加速度0（不使用加减速直接启动），脉冲数3200（16细分下发送3200个脉冲电机转一圈），相对运动
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
			
      //电机到位信号 = 0x03 & 0x02 = 电机到位;
//			Emm_V5_Pos_Control(5, 0, 300, 100, 800, 0, 0);
//			Delay_ms(10);

//			Emm_V5_Pos_Control(6, 0, 300, 100, 200, 0, 0);
//			Delay_ms(10);

//      //等待返回命令，命令数据缓存在数组rxCmd上，长度为rxCount
//			while(rxFrameFlag == false);
//			rxFrameFlag = false;
//			Delay_ms(500);	
//        uint8_t ab;
				  uint8_t cmd[16] = {0};  
					// 装载命令
					cmd[0] =  1;                       // 地址
					cmd[1] =  0x3A;                      
					cmd[2] =  0x6B;                    
//       Beep_ON();
			while(1)
			{					
					// 发送命令
					usart_SendCmd(cmd, 3);
				  Delay_ms(500);
				  if(rxFrameFlag==true)
					{
						 
						if((rxCmd[2]&0x02)==0x02)
						{
							  //电机到位
							USART_SendData(USART2,12);
							Beep_ON();
						}
						 Serial_SendArray_USART2(rxCmd,4);
//						 USART_SendData(USART2,ab);
					   rxFrameFlag=false;
					}
			}			
		}break;
		
		case 2://显示屏测试 二维码测试
		{
			while(Get_Flag==0)
			{
			  Set_ON();//开启二维码扫码模块	
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
		//OLED显示测试
		case 3:
		{
       OLED_ShowNum(1,1,222,3);
		}break;
		case 4://六轴测试
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
		case 5://PCA9685测试
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
		case 6://任务跑法逻辑测试
		{
					
		}break;
		case 7://车整体模块初始化
		{
			Emm_V5_Pos_Control(5, 1, 300, 100, 3200*0.5, 0, 0); 
			Delay_ms(10);	
			
      //位置模式：方向CW，速度1000RPM，加速度0（不使用加减速直接启动），脉冲数3200（16细分下发送3200个脉冲电机转一圈），相对运动
			Emm_V5_Pos_Control(6, 1, 300, 100, 3200*1.4, 0, 0); //下降
			Delay_ms(10);
			
			setPWM(2,0,calculate_PWM(140));	
			Delay_ms(10);			
		}break;
		case 8://电压测试
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
		
	}
}

void TRY(uint8_t i)
{
	switch(i)
	{
		case 1:
		{
	    Set_Go(Back,6,0,400,130);			
		}break;
		case 2:
		{	
			Try(7);
			Delay_ms(1000);
			Set_Go(Left,0.9,0,430,150);

			Delay_ms(2800);
			Set_Go(Front,3,0,400,100);
	
			Try(2);
			Delay_ms(1000);

			Set_Go(Front,3,0,380,100);
			Delay_ms(5000);
			Set_Go(Right,0.3,0,430,150);
	
			Delay_ms(2000);
	
			Emm_V5_Pos_Control(6, 1, 300, 100, 3200*0.3, 0, 0); //下降
			Delay_ms(500);
			setPWM(2,0,calculate_PWM(85));	
			Delay_ms(250);
			Emm_V5_Pos_Control(6, 0, 300, 200, 3200*0.7, 0, 0); //上升
			Delay_ms(10);
			Emm_V5_Pos_Control(5, 0, 300, 100, 3200*0.32, 0, 0); 
			Delay_ms(1000);	
			setPWM(2,0,calculate_PWM(40));	
			Delay_ms(500);

			
			Emm_V5_Pos_Control(6, 0, 300, 100, 3200*0.8, 0, 0); 
			Delay_ms(300);
			Emm_V5_Pos_Control(5, 1, 300, 100, 3200*0.32, 0, 0); 
			Delay_ms(50);
			Set_Go(Left,0.4,0,430,150);
			Delay_ms(2000);
		
			Set_Go(Back,1.7,0,420,120);
			Set_Go(Turn_L,0,90,0,0);
			Delay_ms(3000);
			Set_Go(Front,3.6,0,400,100);
			Delay_ms(5000);
			Set_Go(Front,3.6,0,400,100);
			Delay_ms(5000);
			Set_Go(Turn_L,0,90,0,0);
			Delay_ms(5000);
			Set_Go(Left,0.4,0,430,150);
			Delay_ms(1000);			
			Set_Go(Back,3.3,0,400,120);
			Delay_ms(5000);
			Set_Go(Turn_R,0,90,0,0);
			
		}break;
	}
}


void Run()
{
	
		Set_Go(Left,0.95,0,380,100); 
		Set_Go(Front,3.15,0,380,100); 	
		Try(2);
		runActionGroup(2,1);
		Set_Go(Front,2.842,0,380,100);
		Set_Go(Right,0.28,0,380,170);
		
	  Delay_ms(200);
	  OpenMv_Part(See_Color);		
	  Task_Catch_Work(1);

		Set_Go(Left,0.4,0,380,170);
		Set_Go(Back,1.72,0,380,100);
		Set_Go(Turn_L,0,90,0,0);

		Set_Go(Front,3.5,0,380,100);
		Set_Go(Front,3.5,0,380,100);	
	
		Set_Go(Turn_L,0,90,0,0);
		Set_Go(Right,0.13,0,380,100);
						
	//抵达粗加工区
		runActionGroup(6,1);
		
		Delay_ms(1000);
	  Ring_Adjust();
		
		//抓7-9按顺序抓取  放10-12放123
		PUT(Order_Color_1);

		runActionGroup(2,1);
		
		//抓13-15按顺序抓取 放16-18		
		Catch(Order_Color_1);
		
		Set_Go(Left,0.22,0,380,170);		
		Set_Go(Back,3.18,0,380,120);
		Set_Go(Turn_R,0,90,0,0);
		Set_Go(Back,3.5,0,380,100);
		Set_Go(Right,0.38,0,380,100);
	//抵达暂存区
		runActionGroup(6,1);
		
		Delay_ms(1000);
	  Ring_Adjust();
		
		//抓7-9按顺序抓取  放10-12放123
		PUT(Order_Color_2);

		runActionGroup(2,1);
					
	
	
}

