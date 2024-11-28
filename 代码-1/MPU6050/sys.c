#include "sys.h"

//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI  
void WFI_SET(void)
{
	__ASM volatile("wfi");		  
}
//关闭所有中断
void INTX_DISABLE(void)
{		  
	__ASM volatile("cpsid i");
}
//开启所有中断
void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");		  
}
//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}



void NVIC_Configuration(void)
{
		NVIC_InitTypeDef  NVIC_InitStructure;	
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级

		//////////////////外部中断5优先级配置也就是MPU6050 INT引脚的配置
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;				//使能外部中断通道
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;	//抢占优先级0 
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					//子优先级1
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
		NVIC_Init(&NVIC_InitStructure); 
	
		NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4中断
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;  //先占优先级1级
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;  //从优先级3级
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
		NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00;//抢占优先级2
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;		//子优先级2
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
}

