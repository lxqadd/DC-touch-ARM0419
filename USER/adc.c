/******************** (C) COPYRIGHT 2012 WildFire Team ***************************
 * 文件名  ：adc.c
 * 描述    ：adc应用函数库
 * 实验平台：野火STM32开发板
 * 硬件连接：----------------------------------------------------
 *          |                                                    |
 *          | PC1 - ADC1 连接外部电压(通过一个滑动变阻器分压而来)|
 *          |                                                    |
 *           ----------------------------------------------------
 * 库版本  ：ST3.5.0
 *
 * 作者    ：wildfire team
 * 论坛    ：http://www.amobbs.com/forum-1008-1.html
 * 淘宝    ：http://firestm32.taobao.com
**********************************************************************************/
#include "adc.h"

#define ADC1_DR_Address    ((u32)0x40012400+0x4c)

__IO uint16_t ADC_ConvertedValue;
//__IO u16 ADC_ConvertedValueLocal;
#define N	200
#define M	4

/*
 * 函数名：ADC1_GPIO_Config
 * 描述  ：使能ADC1和DMA1的时钟，初始化PC.01
 * 输入  : 无
 * 输出  ：无
 * 调用  ：内部调用
 */
static void ADC1_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;


    /* Enable ADC1 and GPIOC clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

    /* Configure PC.01  as analog input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;// | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);				// PC1,输入时不用设置速率
}


/* 函数名：ADC1_Mode_Config
 * 描述  ：配置ADC1的工作模式为MDA模式
 * 输入  : 无
 * 输出  ：无
 * 调用  ：内部调用
 */
static void ADC1_Mode_Config(void)
{
    ADC_InitTypeDef ADC_InitStructure;

    /* ADC1 configuration */

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//独立ADC模式
    ADC_InitStructure.ADC_ScanConvMode = ENABLE ; 	 //禁止扫描模式，扫描模式用于多通道采集
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	//开启连续转换模式，即不停地进行ADC转换
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//不使用外部触发转换
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; 	//采集数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = M;	 	//要转换的通道数目1
    ADC_Init(ADC1, &ADC_InitStructure);

    /*配置ADC时钟，为PCLK2的8分频，即9Hz*/
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    /*配置ADC1的通道11为55.	5个采样周期，序列为1 */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_239Cycles5 );
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 2, ADC_SampleTime_239Cycles5 );
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 3, ADC_SampleTime_239Cycles5 );
    ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 4, ADC_SampleTime_239Cycles5 );
    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);

    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);

    /*复位校准寄存器 */
    ADC_ResetCalibration(ADC1);
    /*等待校准寄存器复位完成 */
    while(ADC_GetResetCalibrationStatus(ADC1));

    /* ADC校准 */
    ADC_StartCalibration(ADC1);
    /* 等待校准完成*/
    while(ADC_GetCalibrationStatus(ADC1));

    /* 由于没有采用外部触发，所以使用软件触发ADC转换 */

}

vu16 AD_Value[N][M]; //????ADC????,??DMA?????
vu16 After_filter[M]; //?????????????
void DMA_Configuration(void)
{

    DMA_InitTypeDef DMA_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_DeInit(DMA1_Channel1); //?DMA???1?????????
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR; //DMA??ADC???
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value; //DMA?????
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //????????????
    DMA_InitStructure.DMA_BufferSize = N * M; //DMA???DMA?????
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //?????????
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //?????????
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //?????16?
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //?????16?
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //?????????
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA?? x??????
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //DMA??x????????????
    DMA_Init(DMA1_Channel1, &DMA_InitStructure); //??DMA_InitStruct?????????DMA???

}
/*
 * 函数名：ADC1_Init
 * 描述  ：无
 * 输入  ：无
 * 输出  ：无
 * 调用  ：外部调用
 */
void ADC1_Init(void)
{
    ADC1_GPIO_Config();
    ADC1_Mode_Config();
    DMA_Configuration();
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    DMA_Cmd(DMA1_Channel1, ENABLE);
}


/******************* (C) COPYRIGHT 2012 WildFire Team *****END OF FILE************/

