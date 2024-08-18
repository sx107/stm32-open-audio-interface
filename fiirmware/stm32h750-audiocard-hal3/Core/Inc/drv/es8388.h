/*
 * es8388.h
 *
 *  Created on: 3 февр. 2024 г.
 *      Author: Sx107
 */



#include <main.h>

#define ES8388_ADDR (0x10)

#ifndef ES8388_H_
#define ES8388_H_

typedef enum {ES_LEFT = 0, ES_RIGHT = 1} es_lr_t;

void es8388_init(uint8_t index);

typedef enum {ES_INPUT_1 = 0b00, ES_INPUT_2 = 0b01, ES_INPUT_DIFF = 0b11} es_input_t;
void es8388_set_input(uint8_t index, es_input_t in);

void es8388_set_pga_gain(uint8_t index, es_lr_t lr, float gain); // 0 .. 24db
void es8388_set_pga_gain_both(uint8_t index, float gain_left, float gain_right); // 0 .. 24db

void es8388_set_adc_attenuation(uint8_t index, es_lr_t lr, float atten); // -96 -- 0

typedef enum {ES_DAC_VSPD_D4 = 0, ES_DAC_VSPD_D32 = 1, ES_DAC_VSPD_D64 = 2, ES_DAC_VSPD_D128 = 3, ES_DAC_VSPD_DIS = 4} es_dac_vspd_t;
typedef enum {ES_DAC_GAIN_MUTE = 1, ES_DAC_GAIN_NORM = 0} es_dac_mute_t;
void es8388_set_dac_gain(uint8_t index, es_dac_vspd_t vspd, es_dac_mute_t mute);

void es8388_set_dac_attenuation(uint8_t index, es_lr_t lr, float atten); // -96 -- 0

void es8388_set_output_volume(uint8_t index, uint8_t chan, es_lr_t lr, float vol); // -45 -- 4.5

typedef enum {ES_OUTPUT_NO_DAC = 0, ES_OUTPUT_DAC = 1} es_out_dac_t;
typedef enum {ES_OUTPUT_NO_IN = 0, ES_OUTPUT_IN = 1} es_out_in_t;
void es8388_set_output_mixer(uint8_t index, es_lr_t lr, es_out_dac_t dac, es_out_in_t in, float volume);


#endif /* ES8388_H_ */
