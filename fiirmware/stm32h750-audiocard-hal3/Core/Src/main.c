/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"
#include <drv/usb_timer.h>

#include <drv/clock.h>
#include <drv/mpu.h>
#include <drv/i2c1.h>
#include <drv/sai.h>
#include <drv/es8388.h>
#include <drv/i2s_clock.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/


/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  *
  */


int main(void)
{

	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
	//SCB->VTOR = FLASH_BASE + 0x5000;
	system_init();
	RCC->AHB2ENR |= (RCC_AHB2ENR_D2SRAM1EN | RCC_AHB2ENR_D2SRAM2EN | RCC_AHB2ENR_D2SRAM3EN); // Enable all D2 SRAM1, SRAM2, SRAM3
	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	HAL_Init();

	RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN;
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODE3_Msk, 0b01 << GPIO_MODER_MODE3_Pos);
	MODIFY_REG(GPIOB->OTYPER, GPIO_OTYPER_OT3_Msk, 0b0 << GPIO_OTYPER_OT3_Pos);
	MODIFY_REG(GPIOB->OSPEEDR, GPIO_OSPEEDR_OSPEED3_Msk, 0b11 << GPIO_OSPEEDR_OSPEED3_Pos);

	__DSB();
	SCB_InvalidateDCache();
	SCB_DisableDCache();
	__DSB();
	mpu_init();


	i2c1_init();
	es8388_init(0);
	i2sclock_init();


	sai_init();
	//sai_instance_dma_configure(SAI1);
	i2sclock_select(48000);
	//sai_enable();

	usb_timer_init();
	usb_clock_init();
    MX_USB_DEVICE_Init();


    while (1)
    {
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
