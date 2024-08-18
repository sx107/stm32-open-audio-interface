/*
 * usbd_audio_desc.h
 *
 *  Created on: Aug 15, 2024
 *      Author: Sx107
 */

#ifndef ST_STM32_USB_DEVICE_LIBRARY_CLASS_AUDIO_INC_USBD_AUDIO_DESC_H_
#define ST_STM32_USB_DEVICE_LIBRARY_CLASS_AUDIO_INC_USBD_AUDIO_DESC_H_

#include <main.h>
#include <usbd_def.h>

#define EP_GENERAL										0x01
/*
#define AUDIO_OUT_EP                                  0x01U
#define AUDIO_INTERFACE_DESC_SIZE                     0x09U
#define USB_AUDIO_DESC_SIZ                            0x09U
#define AUDIO_STANDARD_ENDPOINT_DESC_SIZE             0x09U
#define AUDIO_STREAMING_ENDPOINT_DESC_SIZE            0x07U

#define AUDIO_DESCRIPTOR_TYPE                         0x21U
#define USB_DEVICE_CLASS_AUDIO                        0x01U
#define AUDIO_SUBCLASS_AUDIOCONTROL                   0x01U
#define AUDIO_SUBCLASS_AUDIOSTREAMING                 0x02U
#define AUDIO_PROTOCOL_UNDEFINED                      0x00U
#define AUDIO_STREAMING_GENERAL                       0x01U
#define AUDIO_STREAMING_FORMAT_TYPE                   0x02U

#define AUDIO_INTERFACE_DESCRIPTOR_TYPE               0x24U
#define AUDIO_ENDPOINT_DESCRIPTOR_TYPE                0x25U

#define AUDIO_CONTROL_HEADER                          0x01U
#define AUDIO_CONTROL_INPUT_TERMINAL                  0x02U
#define AUDIO_CONTROL_OUTPUT_TERMINAL                 0x03U
#define AUDIO_CONTROL_FEATURE_UNIT                    0x06U

#define AUDIO_INPUT_TERMINAL_DESC_SIZE                0x0CU
#define AUDIO_OUTPUT_TERMINAL_DESC_SIZE               0x09U
#define AUDIO_STREAMING_INTERFACE_DESC_SIZE           0x07U

#define AUDIO_CONTROL_MUTE                            0x0001U

#define AUDIO_FORMAT_TYPE_I                           0x01U
#define AUDIO_FORMAT_TYPE_III                         0x03U

#define AUDIO_ENDPOINT_GENERAL                        0x01U

#define AUDIO_REQ_GET_CUR                             0x81U
#define AUDIO_REQ_SET_CUR                             0x01U

#define AUDIO_OUT_STREAMING_CTRL                      0x02U

#define AUDIO_OUT_TC                                  0x01U
#define AUDIO_IN_TC                                   0x02U
*/

// Feedback
#define AUDIO_48K_FEEDBACK_VALUE											0x60000
#define AUDIO_44K1_FEEDBACK_VALUE											0x58333
#define AUDIO_FEEDBACK_GAIN                           8

#define NUM_SAMPLE_RATES 6
extern const uint32_t sample_rates[6];

// Control
#define AUDIO_MIN_FREQ																44100U
#define AUDIO_MAX_FREQ																96000U
#define	AUDIO_FREQ_RES																1U
#define AUDIO_MIN_VOL																	0U
#define AUDIO_MAX_VOL																	100U
#define AUDIO_VOL_RES																	1U

#define FEEDBACK_HS_BINTERVAL													1U
#define STREAMING_HS_BINTERVAL                        1U

#define AUDIO_WTOTALLENGTH														60U

#define USB_AUDIO_CONFIG_DESC_SIZE                    201U

#define USB_AUDIO_DESC_SIZE                           0x09U

/* Audio Control Interface Descriptor Subtypes */
#define FORMAT_TYPE_I			                           	0x01U

#define AUDIO_REQ_CUR																	0x01U
#define AUDIO_REQ_RANGE																0x02U

#define FEEDBACK_PACKET_SIZE													4U


#define AUDIO_BUFFER_PACKET_NUM												40U
#define AUDIO_BUF_SIZE                        				(AUDIO_BUFFER_PACKET_NUM * (AUDIO_MAX_FREQ / 1000U))

/* Audio20 appendix definitions */
#define AUDIO_FUNCTION 																AUDIO
#define FUNCTION_SUBCLASS_UNDEFINED 									0x00
#define FUNCTION_PROTOCOL_UNDEFINED										0x00
#define AF_VERSION_02_00															IP_VERSION_02_00
#define AUDIO																					0x01
#define INTERFACE_SUBCLASS_UNDEFINED									0x00
#define AUDIOCONTROL																	0x01
#define AUDIOSTREAMING																0x02
#define INTERFACE_PROTOCOL_UNDEFINED									0x00
#define IP_VERSION_02_00															0x20
#define FUNCTION_SUBCLASS_UNDEFINED										0x00

#define CS_UNDEFINED																	0x20
#define CS_DEVICE																			0x21
#define CS_CONFIGURATION															0x22
#define CS_STRING																			0x23
#define CS_INTERFACE																	0x24
#define CS_ENDPOINT																		0x25

#define CS_SAM_FREQ_CONTROL														0x01
#define FU_MUTE_CONTROL																0x01
#define FU_VOLUME_CONTROL															0x02

#define AC_DESCRIPTOR_UNDEFINED												0x00
#define HEADER																				0x01
#define INPUT_TERMINAL																0x02
#define OUTPUT_TERMINAL																0x03
#define CLOCK_SOURCE																	0x0A
#define FEATURE_UNIT																	0x06

#define AS_DESCRIPTOR_UNDEFINED												0x00
#define AS_GENERAL																		0x01
#define FORMAT_TYPE																		0x02

// Interface definitions
#define AC_INTERFACE_NUM															0x00
#define AS_INTERFACE_NUM															0x01
#define ASIN_INTERFACE_NUM															0x02

// Clock source definitions
#define CLOCK_SOURCE_ID																0x04
#define CLOCK_AIN_SOURCE_ID															0x05

// Terminal definitions
#define INPUT_TERMINAL_ID															0x01
#define INPUT_TERMINAL_TYPE														0x0101	// USB Streaming; See Termt20 section 2.1

#define OUTPUT_TERMINAL_ID														0x03
#define OUTPUT_TERMINAL_TYPE													0x0301	// Speaker; See Termt20 section 2.3

#define FEATURE_UNIT_ID																0x02

#define INAUDIO_INPUT_TERMINAL_ID													0x06
#define INAUDIO_INPUT_TERMINAL_TYPE												0x0603

#define INAUDIO_OUTPUT_TERMINAL_ID													0x07
#define INAUDIO_OUTPUT_TERMINAL_TYPE												0x0101

// Endpoint definitions
#define STREAMING_EP_ADDR															0x01
#define STREAMING_EP_ATTRIB														0x05

#define STREAMINGIN_EP_ADDR 													(0x02 | 0x80)
#define STREAMINGIN_EP_ATTRIB														0x05

#define FEEDBACK_EP_ADDR															(STREAMING_EP_ADDR | 0x80)
#define FEEDBACK_EP_ATTRIB														0b010001
//#define FEEDBACK_EP_ATTRIB														0b11

#define STREAMING_EP_NUM 															(STREAMING_EP_ADDR & 0xF)
#define FEEDBACK_EP_NUM 															(FEEDBACK_EP_ADDR & 0xF)
#define STREAMINGIN_EP_NUM															(STREAMINGIN_EP_ADDR & 0xF)

extern __ALIGN_BEGIN uint8_t USBD_AUDIO_CfgDesc[USB_AUDIO_CONFIG_DESC_SIZE] __ALIGN_END;
extern __ALIGN_BEGIN uint8_t USBD_AUDIO_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END;

#endif /* ST_STM32_USB_DEVICE_LIBRARY_CLASS_AUDIO_INC_USBD_AUDIO_DESC_H_ */
