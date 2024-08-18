/*
 * es8388.c
 *
 *  Created on: 3 февр. 2024 г.
 *      Author: Sx107
 */


#include <drv/es8388.h>
#include <drv/i2c1.h>
#include <drv/clock.h>



void es8388_init(uint8_t index) {
	/*
	i2c1_writeRegister8(ES8388_ADDR + index, 0x08, 0);
	i2c1_writeRegister8(ES8388_ADDR + index, 0x02, 0xF3);
	i2c1_writeRegister8(ES8388_ADDR + index, 43, 0x80);
	i2c1_writeRegister8(ES8388_ADDR + index, 0, 0x05);
	i2c1_writeRegister8(ES8388_ADDR + index, 0x01, 0x40);

	i2c1_writeRegister8(ES8388_ADDR + index, 3, 0x00);
	i2c1_writeRegister8(ES8388_ADDR + index, 4, 0x3C);

	i2c1_writeRegister8(ES8388_ADDR + index, 4, 0x3C);

	es8388_set_pga_gain_both(index, 0,0);
	es8388_set_input(index, ES_INPUT_1);
	i2c1_writeRegister8(ES8388_ADDR + index, 12, 0x00);
	i2c1_writeRegister8(ES8388_ADDR + index, 13, 0x02);
    es8388_set_adc_attenuation(index, ES_LEFT, 0);
    es8388_set_adc_attenuation(index, ES_RIGHT, 0);

    i2c1_writeRegister8(ES8388_ADDR + index, 23, 0x00);
    i2c1_writeRegister8(ES8388_ADDR + index, 24, 0x02);

	es8388_set_dac_attenuation(index, ES_LEFT, 0);
	es8388_set_dac_attenuation(index, ES_RIGHT, 0);
	es8388_set_output_mixer(index, ES_LEFT, ES_OUTPUT_DAC, ES_OUTPUT_NO_IN, -15);
	es8388_set_output_mixer(index, ES_RIGHT, ES_OUTPUT_DAC, ES_OUTPUT_NO_IN, -15);
	es8388_set_output_volume(index, 0, ES_LEFT, 0);
	es8388_set_output_volume(index, 0, ES_RIGHT, 0);
	es8388_set_output_volume(index, 1, ES_LEFT, 0);
	es8388_set_output_volume(index, 1, ES_RIGHT, 0);

	i2c1_writeRegister8(ES8388_ADDR + index, 2, 0x00);
	*/


	es8388_set_dac_gain(index, ES_DAC_VSPD_DIS, ES_DAC_GAIN_MUTE);

	i2c1_writeRegister8(ES8388_ADDR + index, 8, 0x00); // Slave mode. Do not touch.

	i2c1_writeRegister8(ES8388_ADDR + index, 2, 0xF3); // Full reset, ADC/DAC reference power down
	_delayMs(1);
	i2c1_writeRegister8(ES8388_ADDR + index, 2, 0x00);

	//i2c1_writeRegister8(ES8388_ADDR + index, 3, 0x08); // ADC power up, input power up, mic down, adc bias up, not low power
	i2c1_writeRegister8(ES8388_ADDR + index, 43, 0x80); // Same LRCLK for ADC and DAC. Do not touch.
	i2c1_writeRegister8(ES8388_ADDR + index, 0, 0b01111110); // Enable VMID, Enable divider
	i2c1_writeRegister8(ES8388_ADDR + index, 1, 0x40); // Power up sequence enable, No low-power mode
	i2c1_writeRegister8(ES8388_ADDR + index, 4, 0x3C); // OUT1, OUT2 enable


	i2c1_writeRegister8(ES8388_ADDR + index, 23, 0b100000); // 24-bit I2S, no LRCLK clock inversion or data swap
	i2c1_writeRegister8(ES8388_ADDR + index, 24, 0x02); // fs*256 MCLK

	es8388_set_dac_attenuation(index, ES_LEFT, 0);
	es8388_set_dac_attenuation(index, ES_RIGHT, 0);

	es8388_set_dac_gain(index, ES_DAC_VSPD_DIS, ES_DAC_GAIN_NORM); // Unmute


	i2c1_writeRegister8(ES8388_ADDR + index, 38, 0b011011); 		// Left/Right 1 after PGA at LMIX/RMIX sel

	es8388_set_output_mixer(index, ES_LEFT, ES_OUTPUT_DAC, ES_OUTPUT_NO_IN, -15);
	es8388_set_output_mixer(index, ES_RIGHT, ES_OUTPUT_DAC, ES_OUTPUT_NO_IN, -15);

	// All outputs 0db volume (actually it's very cool that all output volumes are separate)
	es8388_set_output_volume(index, 0, ES_LEFT, 0);
	es8388_set_output_volume(index, 0, ES_RIGHT, 0);
	es8388_set_output_volume(index, 1, ES_LEFT, 0);
	es8388_set_output_volume(index, 1, ES_RIGHT, 0);


	// Turn off ADC
	i2c1_writeRegister8(ES8388_ADDR + index, 3, 0xFF);

	es8388_set_pga_gain_both(index, 0, 0);
	es8388_set_input(index, ES_INPUT_1);

	// Set ADC settings
    i2c1_writeRegister8(ES8388_ADDR + index, 11, 0x82);
    i2c1_writeRegister8(ES8388_ADDR + index, 12, 0b10000); // 24-bit
    i2c1_writeRegister8(ES8388_ADDR + index, 13, 0x02); // ADCFsMode,single SPEED,RATIO=256, do not touch!

    es8388_set_adc_attenuation(index, ES_LEFT, 0);
    es8388_set_adc_attenuation(index, ES_RIGHT, 0);
    i2c1_writeRegister8(ES8388_ADDR + index, 17, 0x00);

	i2c1_writeRegister8(ES8388_ADDR + index, 3, 0x09); // ADC power up, input power up, mic down, adc bias up, not low power


	i2c1_writeRegister8(ES8388_ADDR + index, 2, 0x00); // Full DAC power-up, no ADC

}

void es8388_set_output_volume(uint8_t index, uint8_t chan, es_lr_t lr, float vol) {
	vol = (vol + 45) / 1.5;
	int16_t v = (int16_t) vol;
	if(v < 0) {v = 0;}
	if(v > 0b100001) {v = 0b100001;}
	i2c1_writeRegister8(ES8388_ADDR + index, 46 + chan + lr, v);
}

void es8388_set_dac_attenuation(uint8_t index, es_lr_t lr, float atten) {
	atten = (atten + 96) / 0.5;
	int16_t v = (int16_t) atten;
	if(v < 0) {v = 0;}
	if(v > 192) {v = 192;}
	v = 192 - v;
	i2c1_writeRegister8(ES8388_ADDR + index, 26 + lr, v);
}

void es8388_set_dac_gain(uint8_t index, es_dac_vspd_t vspd, es_dac_mute_t mute) {
	uint8_t reg = 0;
	reg |= (mute << 1);
	if(vspd != ES_DAC_VSPD_DIS) {
		reg |= vspd << 6;
		reg |= 1 << 5; // enable vspd
	}
	// DACLeR always 0
	i2c1_writeRegister8(ES8388_ADDR + index, 25, reg);
}

void es8388_set_output_mixer(uint8_t index, es_lr_t lr, es_out_dac_t dac, es_out_in_t in, float volume) {
	volume = (6 - volume) * (1/3);
	int16_t v = (int16_t) volume;
	if(v < 0) {v = 0;}
	if(v > 7) {v = 7;}
	uint8_t reg = v;
	reg |= (dac << 7) | (in << 6);
	i2c1_writeRegister8(ES8388_ADDR + index, lr ? 39 : 42, reg);
}

void es8388_set_pga_gain(uint8_t index, es_lr_t lr, float gain) {
	uint8_t reg = i2c1_readRegister8(ES8388_ADDR + index, 9);
	uint8_t initial_reg = reg;
	if(lr == ES_LEFT) {reg &= 0x0F;}
	else {reg &= 0xF0;}

	gain /= 3;
	int16_t v = gain;
	if (v < 0) {v = 0;}
	if (v > 8) {v = 8;}

	reg |= v << (lr == ES_LEFT ? 4 : 0);
	if(reg != initial_reg) {
		i2c1_writeRegister8(ES8388_ADDR + index, 9, reg);
	}
}

void es8388_set_pga_gain_both(uint8_t index, float gain_left, float gain_right) {
	gain_left /= 3;
	int16_t vl = (int16_t)gain_left;
	if (vl < 0) {vl = 0;}
	if (vl > 8) {vl = 8;}

	gain_right /= 3;
	int16_t vr = (int16_t)gain_right;
	if (vr < 0) {vr = 0;}
	if (vr > 8) {vr = 8;}

	i2c1_writeRegister8(ES8388_ADDR + index, 9, (vl << 4) | vr);
}

void es8388_set_input(uint8_t index, es_input_t in) {
	i2c1_writeRegister8(ES8388_ADDR + index, 10, (in << 6) | (in << 4) | (1 << 3));
}

void es8388_set_adc_attenuation(uint8_t index, es_lr_t lr, float atten) {
	atten = (atten + 96) / 0.5;
	int16_t v = (int16_t) atten;
	if(v < 0) {v = 0;}
	if(v > 192) {v = 192;}
	v = 192 - v;
	i2c1_writeRegister8(ES8388_ADDR + index, 16 + lr, v);
}

