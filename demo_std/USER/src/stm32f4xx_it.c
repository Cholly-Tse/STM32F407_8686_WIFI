/**
  ******************************************************************************
  * @file    SPI/SPI_FLASH/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    04-August-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "usart.h"	

/** @addtogroup STM32F4xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup SPI_FLASH
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
	while(1);
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1);
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1);
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1);
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1);
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
	while(1);
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
	while(1);
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
	while(1);
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	while(1);
}
/*******************************************************************************
* Function Name  : WWDG_IRQHandler
* Description    : This function handles WWDG interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void WWDG_IRQHandler(void)
{
	while(1);
}
void PVD_IRQHandler(void)
{
	while(1);
}
void TAMP_STAMP_IRQHandler(void)
{
	while(1);
}
void RTC_WKUP_IRQHandler(void)
{
	while(1);
}
void FLASH_IRQHandler(void)
{
	while(1);
}
void RCC_IRQHandler(void)
{
	while(1);
}
void EXTI0_IRQHandler(void)
{
	while(1);
}
void EXTI1_IRQHandler(void)
{
	while(1);
}
void EXTI2_IRQHandler(void)
{
	while(1);
}
void EXTI3_IRQHandler(void)
{
	while(1);
}
void EXTI4_IRQHandler(void)
{
	while(1);
}
void DMA1_Stream0_IRQHandler(void)
{
	while(1);
}
void DMA1_Stream1_IRQHandler(void)
{
	while(1);
}
void DMA1_Stream2_IRQHandler(void)
{
	while(1);
}
void DMA1_Stream3_IRQHandler(void)
{
	while(1);
}
void DMA1_Stream4_IRQHandler(void)
{
	while(1);
}
void DMA1_Stream5_IRQHandler(void)
{
	while(1);
}
void DMA1_Stream6_IRQHandler(void)
{
	while(1);
}
void ADC_IRQHandler(void)
{
	while(1);
}
void CAN1_TX_IRQHandler(void)
{
	while(1);
}
void CAN1_RX0_IRQHandler(void)
{
	while(1);
}
void CAN1_RX1_IRQHandler(void)
{
	while(1);
}
void CAN1_SCE_IRQHandler(void)
{
	while(1);
}
void EXTI9_5_IRQHandler(void)
{
	while(1);
}
void TIM1_BRK_TIM9_IRQHandler(void)
{
	while(1);
}
void TIM1_UP_TIM10_IRQHandler(void)
{
	while(1);
}
void TIM1_TRG_COM_TIM11_IRQHandler(void)
{
	while(1);
}
void TIM1_CC_IRQHandler(void)
{
	while(1);
}
void TIM2_IRQHandler(void)
{
	while(1);
}
void timer3_int(void);
void TIM3_IRQHandler(void)
{
	timer3_int();
}
void TIM4_IRQHandler(void)
{
	while(1);
}
void I2C1_EV_IRQHandler(void)
{
	while(1);
}
void I2C1_ER_IRQHandler(void)
{
	while(1);
}
void I2C2_EV_IRQHandler(void)
{
	while(1);
}
void I2C2_ER_IRQHandler(void)
{
	while(1);
}
void SPI1_IRQHandler(void)
{
	while(1);
}
void SPI2_IRQHandler(void)
{
	while(1);
}     
void USART1_IRQHandler(void)
{
		u8 Res;

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
		
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}   		 
  } 
}
void USART2_IRQHandler(void)
{
	while(1);
}
void USART3_IRQHandler(void)
{
	while(1);
}          
void EXTI15_10_IRQHandler(void)
{
	while(1);
}
void RTC_Alarm_IRQHandler(void)
{
	while(1);
}
void OTG_FS_WKUP_IRQHandler(void)
{
	while(1);
}
void TIM8_BRK_TIM12_IRQHandler(void)
{
	while(1);
}
void TIM8_UP_TIM13_IRQHandler(void)
{
	while(1);
}
void TIM8_TRG_COM_TIM14_IRQHandler(void)
{
	while(1);
}
void TIM8_CC_IRQHandler(void)
{
	while(1);
}
void DMA1_Stream7_IRQHandler(void)
{
	while(1);
}
void FSMC_IRQHandler(void)
{
	while(1);
}
void stm32_irq(void);
void SDIO_IRQHandler(void)
{
	stm32_irq();
}
void TIM5_IRQHandler(void)
{
	while(1);
}
void SPI3_IRQHandler(void)
{
	while(1);
}
void UART4_IRQHandler(void)
{
	while(1);
}
void UART5_IRQHandler(void)
{
	while(1);
}
void TIM6_DAC_IRQHandler(void)
{
	while(1);
}
void TIM7_IRQHandler(void)
{
	while(1);
}
void DMA2_Stream0_IRQHandler(void)
{
	while(1);
}
void DMA2_Stream1_IRQHandler(void)
{
	while(1);
}
void DMA2_Stream2_IRQHandler(void)
{
	while(1);
}
void DMA2_Stream3_IRQHandler(void)
{
	while(1);
}
void DMA2_Stream4_IRQHandler(void)
{
	while(1);
}
void ETH_IRQHandler(void)
{
	while(1);
}
void ETH_WKUP_IRQHandler(void)
{
	while(1);
}
void CAN2_TX_IRQHandler(void)
{
	while(1);
}
void CAN2_RX0_IRQHandler(void)
{
	while(1);
}
void CAN2_RX1_IRQHandler(void)
{
	while(1);
}
void CAN2_SCE_IRQHandler(void)
{
	while(1);
}
void OTG_FS_IRQHandler(void)
{
	while(1);
}
void DMA2_Stream5_IRQHandler(void)
{
	while(1);
}
void DMA2_Stream6_IRQHandler(void)
{
	while(1);
}
void DMA2_Stream7_IRQHandler(void)
{
	while(1);
}
void I2C3_EV_IRQHandler(void)
{
	while(1);
}
void I2C3_ER_IRQHandler(void)
{
	while(1);
}
void OTG_HS_EP1_OUT_IRQHandler(void)
{
	while(1);
}
void OTG_HS_EP1_IN_IRQHandler(void)
{
	while(1);
}
void OTG_HS_WKUP_IRQHandler(void)
{
	while(1);
}
void OTG_HS_IRQHandler(void)
{
	while(1);
}
void DCMI_IRQHandler(void)
{
	while(1);
}
void CRYP_IRQHandler(void)
{
	while(1);
}                                            
void HASH_RNG_IRQHandler(void)
{
	while(1);
}
void FPU_IRQHandler(void)
{
	while(1);
}

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
