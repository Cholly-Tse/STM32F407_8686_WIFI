#include "sdio_user.h"

void SDIO_Register_Deinit(void)
{
	SDIO->POWER=0x00000000;
	SDIO->CLKCR=0x00000000;
	SDIO->ARG=0x00000000;
	SDIO->CMD=0x00000000;
	SDIO->DTIMER=0x00000000;
	SDIO->DLEN=0x00000000;
	SDIO->DCTRL=0x00000000;
	SDIO->ICR=0x00C007FF;
	SDIO->MASK=0x00000000;	 
}

//初始化SD卡
//返回值:错误代码;(0,无错误)
void SDIO_init(void)
{
 	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_DMA2, ENABLE);//使能GPIOC,GPIOD DMA2时钟
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, ENABLE);//SDIO时钟使能
	
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, ENABLE);//SDIO复位
	
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;//PC8,9,10,11,12复用功能输出	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOC, &GPIO_InitStructure);// PC8,9,10,11,12复用功能输出
	
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2;
	GPIO_Init(GPIOD, &GPIO_InitStructure);//PD2复用功能输出
	
	//引脚复用映射设置
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource8,GPIO_AF_SDIO); //PC8,AF12
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource9,GPIO_AF_SDIO);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_SDIO);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_SDIO);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_SDIO);	
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource2,GPIO_AF_SDIO);	
	
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, DISABLE);//SDIO结束复位
		
 	//SDIO外设寄存器设置为默认值 			   
	SDIO_Register_Deinit();
	
	/*   wifi_en   *///此脚不存在
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
