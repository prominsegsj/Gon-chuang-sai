#include "stm32f10x.h"                  // Device header
#include "stm32f10x_adc.h"
#include "Buzzer.h"
#include "Delay.h"
#include "OLED.h"

void AD_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);    // Enable ADC1 clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);    // Enable GPIOC clock
    
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);                        // ADC clock = 12MHz (72MHz / 6)
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;               // PC3 is connected to ADC1 Channel 13
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);                   
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_55Cycles5);  // ADC channel 13
    
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    ADC_Cmd(ADC1, ENABLE);
    
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1) == SET);
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) == SET);
    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    // Analog Watchdog configuration
    ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_None); 
    ADC_AnalogWatchdogThresholdsConfig(ADC1, 4095, 3700);  // High threshold 4095, Low threshold 3333 (about 10V)
    ADC_AnalogWatchdogSingleChannelConfig(ADC1, ADC_Channel_13);  // Monitor Channel 13 (PC3)
    ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_SingleRegEnable); // Enable watchdog
    
    // Configure interrupt
    NVIC_InitTypeDef NVIC_InitStructure;
    ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

uint16_t AD_GetValue(void)
{
    return ADC_GetConversionValue(ADC1);
}

int AFlag = 0;
uint16_t ADValue;			//AD µº ÷µ
float Voltage;

void ADC1_2_IRQHandler(void)
{
    if (ADC_GetITStatus(ADC1, ADC_IT_AWD) != RESET)
    {
        if (AFlag == 0)
        {
            Beep = ~Beep;  // Toggle beep
            Delay_s(10);    // Beep for 1 second
            Beep = ~Beep;  // Stop beep
						AFlag=1;
        }
				OLED_ShowString(3, 1, "Voltage:0.00V");
        OLED_ShowString(4,1,"Power: Error  ");
				ADValue = AD_GetValue();					
				Voltage = (float)ADValue/325-2.1;
				
				OLED_ShowNum(3, 9, Voltage, 1);				
				OLED_ShowNum(3, 11, (uint16_t)(Voltage * 100) % 100, 2);	
				
        Delay_ms(100);  // Debounce delay
        ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);  // Clear interrupt flag
    }
}
