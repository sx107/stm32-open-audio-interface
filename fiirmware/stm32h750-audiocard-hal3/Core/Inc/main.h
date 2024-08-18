/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include <stm32h7xx.h>
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bits for pre-emption priority
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bits for pre-emption priority
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority
                                                                 1 bits for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority
                                                                 0 bits for subpriority */

#define WAITNBIT(REG, BIT) while(((REG) & (BIT)) == (BIT)) {};
#define WAITBIT(REG, BIT) while(((REG) & (BIT)) != (BIT)) {};
#define WAITBITS(REG, BIT) while((((REG) & (BIT)) & (BIT)) == 0) {};

#define WAITNBITTMO(REG, BIT, TMO) for(uint32_t __tmo = 0; __tmo < TMO; __tmo++) {if(((REG) & (BIT)) != (BIT)) {break;}}
#define WAITBITTMO(REG, BIT, TMO) for(uint32_t __tmo = 0; __tmo < TMO; __tmo++) {if(((REG) & (BIT)) == (BIT)) {break;}}

#define WAITVAL(REG, BIT, VAL) while(((REG) & (BIT)) != (VAL)) {};
#define RETERR(F, E) if(F != 0) {return E;}

#define SIMPLETMO(XTMO) for(uint32_t __tmo = 0; __tmo < XTMO*72/4; __tmo++) {__asm("NOP");}

//#define DMA_BUFFER __attribute__(( section(".SECT_RAM_DMA"), aligned(4) )) volatile
//#define DELAY_BUFFER_1 __attribute__(( section(".SECT_DELAY"), aligned(4) )) volatile
//#define DELAY_BUFFER_2 __attribute__(( section(".SECT_DELAY_2"), aligned(4) )) volatile

#define DMA_BUFFER __attribute__((aligned(32)))

#define GPIO_MODER_MODE_GPI 	(0b00)
#define GPIO_MODER_MODE_GPO 	(0b01)
#define GPIO_MODER_MODE_AF		(0b10)
#define GPIO_MODER_MODE_ANALOG 	(0b11)

#define PACK_DATA(_ptr, _type, _value) do{\
	*(_type*)_ptr = (_value); \
	_ptr += sizeof(_type); } while(0)

#define ABS(X) (X < 0 ? -X : X)
#define CLAMP(x, y, z) ((x) < (y) ? (y) : (x) > (z) ? (z) : (x))

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
