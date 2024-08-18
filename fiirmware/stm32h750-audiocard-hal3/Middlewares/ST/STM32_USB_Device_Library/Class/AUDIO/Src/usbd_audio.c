/**
  ******************************************************************************
  * @file    usbd_audio.c
  * @author  MCD Application Team
  * @brief   This file provides the Audio core functions.
  *
  * @verbatim
  *
  *          ===================================================================
  *                                AUDIO Class  Description
  *          ===================================================================
  *           This driver manages the Audio Class 1.0 following the "USB Device Class Definition for
  *           Audio Devices V1.0 Mar 18, 98".
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Standard AC Interface Descriptor management
  *             - 1 Audio Streaming Interface (with single channel, PCM, Stereo mode)
  *             - 1 Audio Streaming Endpoint
  *             - 1 Audio Terminal Input (1 channel)
  *             - Audio Class-Specific AC Interfaces
  *             - Audio Class-Specific AS Interfaces
  *             - AudioControl Requests: only SET_CUR and GET_CUR requests are supported (for Mute)
  *             - Audio Feature Unit (limited to Mute control)
  *             - Audio Synchronization type: Asynchronous
  *             - Single fixed audio sampling rate (configurable in usbd_conf.h file)
  *          The current audio class version supports the following audio features:
  *             - Pulse Coded Modulation (PCM) format
  *             - sampling rate: 48KHz.
  *             - Bit resolution: 16
  *             - Number of channels: 2
  *             - No volume control
  *             - Mute/Unmute capability
  *             - Asynchronous Endpoints
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
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

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
- "stm32xxxxx_{eval}{discovery}_audio.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio.h"
#include "usbd_ctlreq.h"
#include "usbd_audio_desc.h"
#include "usbd_def.h"
#include "string.h"

#include <drv/sai.h>
#include <drv/usb_timer.h>


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_AUDIO
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_AUDIO_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_Macros
  * @{
  */
#define AUDIO_SAMPLE_FREQ(frq)         (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

#define AUDIO_PACKET_SZE(frq)          (uint8_t)(((frq * 2U * 2U)/1000U) & 0xFFU), \
                                       (uint8_t)((((frq * 2U * 2U)/1000U) >> 8) & 0xFFU)

/**
  * @}
  */


/** @defgroup USBD_AUDIO_Private_FunctionPrototypes
  * @{
  */
static uint8_t USBD_AUDIO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_AUDIO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);

static uint8_t USBD_AUDIO_Setup(USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req);

static uint8_t *USBD_AUDIO_GetCfgDesc(uint16_t *length);
static void *USBD_AUDIO_GetAudioHeaderDesc(uint8_t *pConfDesc);
static uint8_t *USBD_AUDIO_GetDeviceQualifierDesc(uint16_t *length);
static uint8_t USBD_AUDIO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_AUDIO_EP0_TxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_AUDIO_SOF(USBD_HandleTypeDef *pdev);

static uint8_t USBD_AUDIO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_AUDIO_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static void AUDIO_REQ_GetRange(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);



/**
  * @}
  */

/** @defgroup USBD_AUDIO_Private_Variables
  * @{
  */

USBD_ClassTypeDef USBD_AUDIO =
{
  USBD_AUDIO_Init,
  USBD_AUDIO_DeInit,
  USBD_AUDIO_Setup,
  USBD_AUDIO_EP0_TxReady,
  USBD_AUDIO_EP0_RxReady,
  USBD_AUDIO_DataIn,
  USBD_AUDIO_DataOut,
  USBD_AUDIO_SOF,
  USBD_AUDIO_IsoINIncomplete,
  USBD_AUDIO_IsoOutIncomplete,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetCfgDesc,
  USBD_AUDIO_GetDeviceQualifierDesc,
};




/**
  * @}
  */

/** @defgroup USBD_AUDIO_Private_Functions
  * @{
  */

/**
  * @brief  USBD_AUDIO_Init
  *         Initialize the AUDIO interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */

DMA_BUFFER USBD_AUDIO_HandleTypeDef s_audio;

static uint8_t USBD_AUDIO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  USBD_AUDIO_HandleTypeDef *haudio;

  /* Allocate Audio structure */
  haudio = &s_audio;
  if (haudio == NULL)
  {
    pdev->pClassData = NULL;
    return (uint8_t)USBD_EMEM;
  }

  pdev->pClassData = (void *)haudio;

   pdev->ep_out[STREAMING_EP_NUM].bInterval = STREAMING_HS_BINTERVAL;
   pdev->ep_out[FEEDBACK_EP_NUM].bInterval = FEEDBACK_HS_BINTERVAL;

  /* Open EP OUT */
  (void)USBD_LL_OpenEP(pdev, STREAMING_EP_ADDR, USBD_EP_TYPE_ISOC, USB_HS_MAX_PACKET_SIZE);
  (void)USBD_LL_OpenEP(pdev, FEEDBACK_EP_ADDR, USBD_EP_TYPE_ISOC, FEEDBACK_PACKET_SIZE);
  pdev->ep_out[STREAMING_EP_NUM].is_used = 1U;
  pdev->ep_in[FEEDBACK_EP_NUM].is_used = 1U;
  USBD_LL_FlushEP(pdev, STREAMING_EP_ADDR);
  USBD_LL_FlushEP(pdev, FEEDBACK_EP_ADDR);
  //pdev->ep_out[STREAMING_EP_NUM].maxpacket = 512;

  haudio->alt_setting = 0U;
  haudio->sam_freq=48000;
  haudio->feedback_base = 0x60000;
  haudio->feedback_value = 0x60000;
  haudio->feedback_base_mult = 1;
  haudio->state = AUDIO_STATE_STOPPED;

  /* Initialize the Audio output Hardware layer */
  if (((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->Init() != 0U)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Prepare Out endpoint to receive 1st packet */
  (void)USBD_LL_PrepareReceive(pdev, STREAMING_EP_ADDR, (uint8_t*)haudio->packet_buffer, USB_HS_MAX_PACKET_SIZE);

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_AUDIO_Init
  *         DeInitialize the AUDIO layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_AUDIO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);

  /* Open EP OUT */
  (void)USBD_LL_CloseEP(pdev, STREAMING_EP_NUM);

  pdev->ep_out[STREAMING_EP_NUM].is_used = 0U;
  pdev->ep_out[STREAMING_EP_NUM].bInterval = 0U;
  pdev->ep_in[FEEDBACK_EP_NUM].is_used = 0U;
  pdev->ep_in[FEEDBACK_EP_NUM].bInterval = 0U;

  /* DeInit  physical Interface components */
  if (pdev->pClassData != NULL)
  {
    ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData)->DeInit(0U);
    //(void)USBD_free(pdev->pClassData);
    //pdev->pClassData = NULL;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_AUDIO_Setup
  *         Handle the AUDIO specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_AUDIO_Setup(USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req)
{
	  USBD_AUDIO_HandleTypeDef* haudio = pdev->pClassData;
	  uint16_t len;
	  uint8_t *pbuf;
	  uint16_t status_info = 0U;

	  switch (req->bmRequest & USB_REQ_TYPE_MASK)
	  {
	    case USB_REQ_TYPE_CLASS:


	    	switch ((req->bRequest) & 0x7F)
	    	{
					case AUDIO_REQ_CUR:
						if (req->bmRequest & 0x80)
						{
							AUDIO_REQ_GetCurrent(pdev, req);
						}
						else
						{
							AUDIO_REQ_SetCurrent(pdev, req);
						}
						break;

					case AUDIO_REQ_RANGE:
						if (req->bmRequest & 0x80)
						{
							AUDIO_REQ_GetRange(pdev, req);
						}
						else
						{
							goto ret_err;
						}
						break;

					default:
						goto ret_err;
						break;
	    	}
	      break;

	    case USB_REQ_TYPE_STANDARD:
	      switch ((req->bRequest) & 0x7F)
	      {
	        case USB_REQ_GET_STATUS:
	          if (pdev->dev_state == USBD_STATE_CONFIGURED)
	          {
	            USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
	          }
	          else
	          {
	          	goto ret_err;
	          }
	          break;

	        case USB_REQ_GET_DESCRIPTOR:
	          if (HIBYTE(req->wValue) == CS_DEVICE)
	          {
	          	pbuf = (uint8_t *)USBD_AUDIO_GetAudioHeaderDesc(pdev->pConfDesc);
	            if (pbuf != NULL)
	            {
	              len = MIN(USB_AUDIO_DESC_SIZE, req->wLength);
	              USBD_CtlSendData(pdev, pbuf, len);
	            }
	            else
	            {
	            	goto ret_err;
	            }
	          }
	          break;

	        case USB_REQ_GET_INTERFACE:
	          if (pdev->dev_state == USBD_STATE_CONFIGURED)
	          {
	            USBD_CtlSendData(pdev, (uint8_t *)&haudio->alt_setting, 1U);
	          }
	          else
	          {
	          	goto ret_err;
	          }
	          break;

	        case USB_REQ_SET_INTERFACE:
	          if (pdev->dev_state == USBD_STATE_CONFIGURED)
	          {
	            if ((uint8_t)(req->wValue) <= USBD_MAX_NUM_INTERFACES)
	            {
	              haudio->alt_setting = (uint8_t)(req->wValue);
	              haudio->bit_depth = (haudio->alt_setting == 1) ? 32U : 24U;
	            }
	            else
	            {
	            	goto ret_err;
	            }
	          }
	          else
	          {
	          	goto ret_err;
	          }
	          break;

	        case USB_REQ_CLEAR_FEATURE:
	          break;

	        default:
	          goto ret_err;
	          break;
	      }
	      break;

	    default:
	    	goto ret_err;
	      break;
	  }

	  return USBD_OK;

	  ret_err:
	  	USBD_CtlError(pdev, req);
	    return USBD_FAIL;
}


/**
  * @brief  USBD_AUDIO_GetCfgDesc
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_AUDIO_GetCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_AUDIO_CfgDesc);

  return USBD_AUDIO_CfgDesc;
}

/**
  * @brief  USBD_AUDIO_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_AUDIO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	USBD_AUDIO_HandleTypeDef* haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

	if (epnum == FEEDBACK_EP_NUM)
	{
		float sf = sai_feedback;
		sf = CLAMP(sf, -50000, 50000);
		haudio->feedback_value = haudio->feedback_base/* + sf */;
		USBD_LL_Transmit(pdev, FEEDBACK_EP_ADDR, (uint8_t*)&haudio->feedback_value, FEEDBACK_PACKET_SIZE);
	}

  /* Only OUT data are processed */
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_AUDIO_EP0_RxReady
  *         handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
	USBD_AUDIO_HandleTypeDef* haudio = pdev->pClassData;
	USBD_AUDIO_ItfTypeDef* itf = pdev->pUserData;

	switch (haudio->control.unit)
	{
	case CLOCK_SOURCE_ID:
		if (haudio->control.cmd == CS_SAM_FREQ_CONTROL)
		{
			haudio->sam_freq = *(uint32_t*)haudio->control.data;

			// Set buffer capacity
			uint32_t packetSize = (haudio->sam_freq % 48000U == 0) ?
					(haudio->sam_freq / 1000U) :
					(haudio->sam_freq / 147U * 160U / 1000U);

			uint32_t multiplier;
			if (haudio->sam_freq % 44100U) {
					multiplier = haudio->sam_freq / 48000U;
					haudio->feedback_base = multiplier * AUDIO_48K_FEEDBACK_VALUE;
					haudio->feedback_base_mult = multiplier;
			} else {
				multiplier = haudio->sam_freq / 44100U;
				haudio->feedback_base = multiplier * AUDIO_44K1_FEEDBACK_VALUE;
				haudio->feedback_base_mult = multiplier;
			}

			itf->AudioCmd(haudio->control.data, haudio->control.len, AUDIO_CMD_FREQ);
		}
		else
		{
			return USBD_FAIL;
		}
		break;

	case FEATURE_UNIT_ID:
		switch (haudio->control.cmd)
		{
		case FU_MUTE_CONTROL:
			itf->AudioCmd(haudio->control.data, haudio->control.len, AUDIO_CMD_MUTE);
			break;

		case FU_VOLUME_CONTROL:
			itf->AudioCmd(haudio->control.data, haudio->control.len, AUDIO_CMD_VOLUME);
			break;

		default:
			return USBD_FAIL;
			break;
		}
		break;

	default:
		return USBD_FAIL;
		break;
	}
  return USBD_OK;
}
/**
  * @brief  USBD_AUDIO_EP0_TxReady
  *         handle EP0 TRx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_AUDIO_EP0_TxReady(USBD_HandleTypeDef *pdev)
{
  UNUSED(pdev);

  /* Only OUT control data are processed */
  return (uint8_t)USBD_OK;
}
/**
  * @brief  USBD_AUDIO_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */

uint32_t errors = 0;
uint16_t pcnt[300];
uint8_t incomplete_packet[300];
uint16_t pcnt_pos = 0;



static uint8_t USBD_AUDIO_SOF(USBD_HandleTypeDef *pdev)
{

  static uint8_t first_counter = 0;
  USBD_AUDIO_HandleTypeDef* haudio = pdev->pClassData;
  USBD_AUDIO_ItfTypeDef* itf = pdev->pUserData;

  haudio->feedback_base = usb_fb_value() * haudio->feedback_base_mult;

  if (first_counter < 100) {
	  USBD_LL_Transmit(pdev, FEEDBACK_EP_ADDR, (uint8_t*)&haudio->feedback_value, FEEDBACK_PACKET_SIZE);
	  first_counter++;
  }



  if(pcnt_pos < 299) {pcnt_pos += 1;}
  else {
	  for(uint16_t i = 0; i < 300; i++) {
 		  incomplete_packet[i] = 0;
		  pcnt[i] = 0;
	  }
	  pcnt_pos = 0;
  }


  if(haudio->skipped_packets >= 1) {
	  errors += 1;
  }

  if(haudio->state == AUDIO_STATE_PLAYING) {
	  haudio->skipped_packets++;
  }

	if(haudio->skipped_packets >= MAX_SKIPPED_PACKETS && haudio->state == AUDIO_STATE_PLAYING) {
		haudio->state = AUDIO_STATE_STOPPED;
		haudio->played_packets = 0;
		haudio->skipped_packets = 0;
		itf->AudioCmd(0, 0, AUDIO_CMD_STOP);
	} else if (haudio->played_packets >= PACKETS_PLAY_SHIFT && haudio->state == AUDIO_STATE_STOPPED) {
		haudio->state = AUDIO_STATE_PLAYING;
		haudio->played_packets = 0;
		itf->AudioCmd(0, 0, AUDIO_CMD_PLAY);
	}


  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_AUDIO_IsoINIncomplete
  *         handle data ISO IN Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_AUDIO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	USBD_AUDIO_HandleTypeDef* haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

	if (epnum == FEEDBACK_EP_NUM)
	{
		USBD_LL_Transmit(pdev, FEEDBACK_EP_ADDR, (uint8_t*)&haudio->feedback_value, FEEDBACK_PACKET_SIZE);
	}

  return (uint8_t)USBD_OK;
}
/**
  * @brief  USBD_AUDIO_IsoOutIncomplete
  *         handle data ISO OUT Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_AUDIO_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	  USBD_AUDIO_HandleTypeDef *haudio;
	  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

	  incomplete_packet[pcnt_pos] += 1;

  //if (epnum == STREAMING_EP_NUM)
  //{
	  //uint16_t PacketSize;
	  //PacketSize = (uint16_t)USBD_LL_GetRxDataSize(pdev, 1);
	  //USBD_LL_FlushEP(pdev, STREAMING_EP_NUM);
	  (void)USBD_LL_PrepareReceive(pdev, STREAMING_EP_NUM, (uint8_t*)&haudio->packet_buffer, USB_HS_MAX_PACKET_SIZE);
  //}

  return (uint8_t)USBD_OK;
}
/**
  * @brief  USBD_AUDIO_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */



static uint8_t USBD_AUDIO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	GPIOB->BSRR |= GPIO_BSRR_BS3;
  uint16_t PacketSize;
  USBD_AUDIO_HandleTypeDef *haudio;

  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassData;

  if (haudio == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (epnum == STREAMING_EP_NUM)
  {

	PacketSize = (uint16_t)USBD_LL_GetRxDataSize(pdev, epnum);
	(void)USBD_LL_PrepareReceive(pdev, STREAMING_EP_NUM, (uint8_t*)&(haudio->packet_buffer), USB_HS_MAX_PACKET_SIZE);
	pcnt[pcnt_pos] += PacketSize;

	haudio->skipped_packets = 0;
	if(haudio->state == AUDIO_STATE_STOPPED && PacketSize != 0) {haudio->played_packets += 1;}

	if(sai_wrpos + PacketSize / 4 <= BUFFER_SIZE) {
		memcpy(&(_sai_outBuf[sai_wrpos]), haudio->packet_buffer, PacketSize);
	} else {
		uint8_t cpy = BUFFER_SIZE - sai_wrpos;
		memcpy(&(_sai_outBuf[sai_wrpos]), haudio->packet_buffer, cpy * 4);
		memcpy(&(_sai_outBuf[0]), haudio->packet_buffer + cpy, PacketSize - cpy * 4);
	}

	sai_wrpos += PacketSize / 4;
	if(sai_wrpos >= BUFFER_SIZE) {sai_wrpos -= BUFFER_SIZE; sai_laps_difference -= 1;}

  }
  GPIOB->BSRR |= GPIO_BSRR_BR3;
  return (uint8_t)USBD_OK;
}

/**
  * @brief  AUDIO_Req_GetCurrent
  *         Handles the GET_CUR Audio control request.
  * @param  pdev: instance
  * @param  req: setup class request
  * @retval status
  */
static void *USBD_AUDIO_GetAudioHeaderDesc(uint8_t *pConfDesc)
{
  USBD_ConfigDescTypeDef *desc = (USBD_ConfigDescTypeDef *)(void *)pConfDesc;
  USBD_DescHeaderTypeDef *pdesc = (USBD_DescHeaderTypeDef *)(void *)pConfDesc;
  uint8_t *pAudioDesc =  NULL;
  uint16_t ptr;

  if (desc->wTotalLength > desc->bLength)
  {
    ptr = desc->bLength;

    while (ptr < desc->wTotalLength)
    {
      pdesc = USBD_GetNextDesc((uint8_t *)pdesc, &ptr);
      if ((pdesc->bDescriptorType == CS_INTERFACE) &&
          (pdesc->bDescriptorSubType == HEADER))
      {
        pAudioDesc = (uint8_t *)pdesc;
        break;
      }
    }
  }
  return pAudioDesc;
}


static void AUDIO_REQ_GetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef* haudio = pdev->pClassData;

  if (haudio == NULL)
  {
    return;
  }

  USBD_memset(haudio->control.data, 0, USB_MAX_EP0_SIZE);

  switch (HIBYTE(req->wIndex))
  {
  case FEATURE_UNIT_ID:
  	break;

  case CLOCK_SOURCE_ID:
  	break;

  case CLOCK_AIN_SOURCE_ID:
	  break;

  default:
  	USBD_CtlError(pdev, req);
  	break;
  }

  USBD_CtlSendData(pdev, haudio->control.data, MIN(req->wLength, USB_MAX_EP0_SIZE));
}
static void AUDIO_REQ_SetCurrent(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef* haudio = pdev->pClassData;

  if (haudio == NULL)
  {
    return;
  }

  if (req->wLength != 0U)
  {
  	haudio->control.cmd = HIBYTE(req->wValue);
  	haudio->control.len = (uint8_t)MIN(req->wLength, USB_MAX_EP0_SIZE);
  	haudio->control.unit = HIBYTE(req->wIndex);

  	USBD_CtlPrepareRx(pdev, haudio->control.data, haudio->control.len);
  }
}
static void AUDIO_REQ_GetRange(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	USBD_AUDIO_HandleTypeDef* haudio = pdev->pClassData;

	if (haudio == NULL)
	{
		return;
	}

	USBD_memset(haudio->control.data, 0, USB_MAX_EP0_SIZE);

	uint8_t* pbuf = haudio->control.data;

	switch (HIBYTE(req->wIndex))
	{
	case CLOCK_SOURCE_ID:
		if (HIBYTE(req->wValue) == CS_SAM_FREQ_CONTROL)
		{

			PACK_DATA(pbuf, uint16_t, 1U);
			PACK_DATA(pbuf, uint32_t, AUDIO_MIN_FREQ);
			PACK_DATA(pbuf, uint32_t, AUDIO_MAX_FREQ);
			PACK_DATA(pbuf, uint32_t, AUDIO_FREQ_RES);
			/*
			PACK_DATA(pbuf, uint16_t, NUM_SAMPLE_RATES);
			for(uint8_t i = 0; i < NUM_SAMPLE_RATES; i++) {
				PACK_DATA(pbuf, uint32_t, sample_rates[i]);
				PACK_DATA(pbuf, uint32_t, sample_rates[i]);
				PACK_DATA(pbuf, uint32_t, 0);
			}
			*/
		}
		else
		{
			USBD_CtlError(pdev, req);
			return;
		}
		break;

	case CLOCK_AIN_SOURCE_ID:
		if (HIBYTE(req->wValue) == CS_SAM_FREQ_CONTROL)
		{

			PACK_DATA(pbuf, uint16_t, 1U);
			PACK_DATA(pbuf, uint32_t, AUDIO_MIN_FREQ);
			PACK_DATA(pbuf, uint32_t, AUDIO_MAX_FREQ);
			PACK_DATA(pbuf, uint32_t, AUDIO_FREQ_RES);
			/*

			PACK_DATA(pbuf, uint16_t, NUM_SAMPLE_RATES);
			for(uint8_t i = 0; i < NUM_SAMPLE_RATES; i++) {
				PACK_DATA(pbuf, uint32_t, sample_rates[i]);
				PACK_DATA(pbuf, uint32_t, sample_rates[i]);
				PACK_DATA(pbuf, uint32_t, 0);
			}
			*/

		}
		else
		{
			USBD_CtlError(pdev, req);
			return;
		}
		break;

	case FEATURE_UNIT_ID:
		if (HIBYTE(req->wValue) == FU_VOLUME_CONTROL)
		{
			PACK_DATA(pbuf, uint16_t, 1U);
			PACK_DATA(pbuf, uint16_t, AUDIO_MIN_VOL);
			PACK_DATA(pbuf, uint16_t, AUDIO_MAX_VOL);
			PACK_DATA(pbuf, uint16_t, AUDIO_VOL_RES);
		}
		else
		{
			USBD_CtlError(pdev, req);
			return;
		}
		break;

	default:
		USBD_CtlError(pdev, req);
		return;
		break;
	}

	USBD_CtlSendData(pdev, haudio->control.data, MIN(req->wLength, USB_MAX_EP0_SIZE));
}


/**
  * @brief  DeviceQualifierDescriptor
  *         return Device Qualifier descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_AUDIO_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_AUDIO_DeviceQualifierDesc);

  return USBD_AUDIO_DeviceQualifierDesc;
}

/**
  * @brief  USBD_AUDIO_RegisterInterface
  * @param  fops: Audio interface callback
  * @retval status
  */
uint8_t USBD_AUDIO_RegisterInterface(USBD_HandleTypeDef *pdev,
                                     USBD_AUDIO_ItfTypeDef *fops)
{
  if (fops == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  pdev->pUserData = fops;

  return (uint8_t)USBD_OK;
}
/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
