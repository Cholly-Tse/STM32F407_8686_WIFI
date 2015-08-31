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

//��ʼ��SD��
//����ֵ:�������;(0,�޴���)
void SDIO_init(void)
{
 	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_DMA2, ENABLE);//ʹ��GPIOC,GPIOD DMA2ʱ��
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, ENABLE);//SDIOʱ��ʹ��
	
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, ENABLE);//SDIO��λ
	
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;//PC8,9,10,11,12���ù������	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOC, &GPIO_InitStructure);// PC8,9,10,11,12���ù������
	
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2;
	GPIO_Init(GPIOD, &GPIO_InitStructure);//PD2���ù������
	
	//���Ÿ���ӳ������
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource8,GPIO_AF_SDIO); //PC8,AF12
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource9,GPIO_AF_SDIO);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_SDIO);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_SDIO);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_SDIO);	
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource2,GPIO_AF_SDIO);	
	
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, DISABLE);//SDIO������λ
		
 	//SDIO����Ĵ�������ΪĬ��ֵ 			   
	SDIO_Register_Deinit();
	
	/*   wifi_en   *///�˽Ų�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
