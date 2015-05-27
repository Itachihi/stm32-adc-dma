#include "stm32f10x_rcc.h"
#include "readvb.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "stm32f10x_dma.h"

#define  __TIMES__    20
#define ADC1_DR_Address ((u32)0x4001244C)  //数据寄存器ADC_DR address
__IO uint16_t AD_Value[2]={0};
/********adc gpio config  ---PA5 6 7 ****/
static void ADC1_GPI0_Config() {
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static  void ADC1_Mode_Config(void) {
	ADC_InitTypeDef ADC_InitStructure;

	ADC_DeInit(ADC1);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;  //多通道采集
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  //连续转换
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 2;  //通道数目	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 1, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 2, ADC_SampleTime_239Cycles5 );
	ADC_Init(ADC1, &ADC_InitStructure);
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);  //ADC CLK  9Hz
	ADC_DMACmd(ADC1, ENABLE);  
	ADC_Cmd(ADC1, ENABLE);
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1));

}

static void DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;   
		
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); 
	DMA_DeInit(DMA1_Channel1);   
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;  
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)AD_Value;      
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;              
	DMA_InitStructure.DMA_BufferSize = 2;                                          
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;   
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;        
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;   
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;      
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;   
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;   
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);   
	       
	DMA_Cmd(DMA1_Channel1, ENABLE);   
}

void ADC1_Init(void) {
	ADC1_GPI0_Config();
	ADC1_Mode_Config();
	DMA_Config();
	ADC_SoftwareStartConvCmd(ADC1, ENABLE); 
}
uint16_t readVB(void) {
	uint8_t i = 0;
	uint32_t vref = 0, v = 0;

	for (i = 0; i < __TIMES__; ++i) {
		vref+=AD_Value[0];
		v +=AD_Value[1];
	}
	printf("v %d vref %d\r\n",v,vref);
	v = 11 * v * 2500 / vref;   //基准电压2.5V  分压1/11
	printf("v is %d \r\n", v);

	return v;
}
