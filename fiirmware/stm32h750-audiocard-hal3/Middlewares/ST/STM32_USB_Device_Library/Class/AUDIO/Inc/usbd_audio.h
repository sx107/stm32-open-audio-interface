/**
  ******************************************************************************
  * @file    usbd_audio.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_audio.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_AUDIO_H
#define __USB_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_AUDIO
  * @brief This file is the Header file for usbd_audio.c
  * @{
  */




#define MAX_SKIPPED_PACKETS 5
#define PACKETS_PLAY_SHIFT 1

/* Number of sub-packets in the audio transfer buffer. You can modify this value but always make sure
  that it is an even number and higher than 3 */
//#define AUDIO_OUT_PACKET_NUM                          80U
/* Total size of the audio transfer buffer */
//#define AUDIO_TOTAL_BUF_SIZE                          ((uint16_t)(AUDIO_OUT_PACKET * AUDIO_OUT_PACKET_NUM))

/* Audio Commands enumeration */
typedef enum
{
  AUDIO_CMD_PLAY,
  AUDIO_CMD_STOP,
	AUDIO_CMD_FREQ,
	AUDIO_CMD_MUTE,
	AUDIO_CMD_VOLUME,
} AUDIO_CommandTypeDef;

typedef enum
{
	AUDIO_STATE_STOPPED,
	AUDIO_STATE_PLAYING,
} AUDIO_StateTypeDef;
/**
  * @}
  */


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */
typedef struct
{
  uint8_t data[USB_MAX_EP0_SIZE];
  uint8_t cmd;
  uint8_t len;
  uint8_t unit;
} __attribute__((aligned(32))) USBD_AUDIO_ControlTypeDef;


typedef struct
{
  int32_t packet_buffer[128];
  uint32_t sam_freq;
  uint32_t feedback_base;
  uint32_t feedback_value;
  USBD_AUDIO_ControlTypeDef control;
  float feedback_base_mult;
  uint8_t bit_depth;
  uint32_t alt_setting;


  // Play / stop control
  AUDIO_StateTypeDef state;
  uint32_t skipped_packets;
  uint32_t played_packets;
} __attribute__((aligned(32))) USBD_AUDIO_HandleTypeDef;



typedef struct
{
  int8_t (*Init)();
  int8_t (*DeInit)();
  int8_t (*AudioCmd)(uint8_t *pbuf, uint32_t size, uint8_t cmd);
} USBD_AUDIO_ItfTypeDef;
/**
  * @}
  */



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */

extern USBD_ClassTypeDef USBD_AUDIO;
#define USBD_AUDIO_CLASS &USBD_AUDIO
/**
  * @}
  */

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t USBD_AUDIO_RegisterInterface(USBD_HandleTypeDef *pdev,
                                     USBD_AUDIO_ItfTypeDef *fops);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USB_AUDIO_H */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
