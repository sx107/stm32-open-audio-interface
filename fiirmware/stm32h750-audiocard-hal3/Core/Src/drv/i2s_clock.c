/*
 * i2s_clock.c
 *
 *  Created on: Aug 8, 2024
 *      Author: Sx107
 */


#include <main.h>
#include <drv/i2s_clock.h>
#include <drv/sai.h>

void i2sclock_init() {
	RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN;
	MODIFY_REG(GPIOC->MODER, GPIO_MODER_MODE9_Msk, 0b10 << GPIO_MODER_MODE9_Pos);
	MODIFY_REG(GPIOC->OTYPER, GPIO_OTYPER_OT9_Msk, 0b0 << GPIO_OTYPER_OT9_Pos);
	MODIFY_REG(GPIOC->OSPEEDR, GPIO_OSPEEDR_OSPEED9_Msk, 0b11 << GPIO_OSPEEDR_OSPEED9_Pos);
	MODIFY_REG(GPIOC->AFR[1], GPIO_AFRH_AFSEL9_Msk, 5 << GPIO_AFRH_AFSEL9_Pos);

	MODIFY_REG(GPIOC->MODER, GPIO_MODER_MODE8_Msk, 0b01 << GPIO_MODER_MODE8_Pos);
	MODIFY_REG(GPIOC->OTYPER, GPIO_OTYPER_OT8_Msk, 0b0 << GPIO_OTYPER_OT8_Pos);
	MODIFY_REG(GPIOC->OSPEEDR, GPIO_OSPEEDR_OSPEED8_Msk, 0b11 << GPIO_OSPEEDR_OSPEED8_Pos);

	GPIOC->BSRR |= GPIO_BSRR_BS8;

	RCC->D2CCIP1R |= 0b011 << RCC_D2CCIP1R_SAI1SEL_Pos;
	RCC->D2CCIP1R |= 0b011 << RCC_D2CCIP1R_SAI23SEL_Pos;
	RCC->D3CCIPR |= 0b011 << RCC_D3CCIPR_SAI4ASEL_Pos;
	RCC->D3CCIPR |= 0b011 << RCC_D3CCIPR_SAI4BSEL_Pos;
}
void i2sclock_select(uint32_t freq) {
	uint8_t div = 4;
	if (freq < 11025 || freq > 192000) {return;}
	if(freq % 12000 == 0) {
		GPIOC->BSRR |= GPIO_BSRR_BS8;
		div = 192000 / freq;
	} else if (freq % 11025 == 0) {
		GPIOC->BSRR |= GPIO_BSRR_BR8;
		div = 176400 / freq;
	} else {
		return;
	}

	if(div == 1) {div = 0;}
	//else {div -= 1;}

	//HAL_SAI_InitProtocol(hsai, protocol, datasize, nbslot)

	uint8_t sai_enabled = SAI1_Block_A->CR1 & SAI_xCR1_SAIEN ? 1 : 0;
	if (sai_enabled) {sai_disable();}
	MODIFY_REG(SAI1_Block_A->CR1, SAI_xCR1_MCKDIV_Msk, div << SAI_xCR1_MCKDIV_Pos);
	MODIFY_REG(SAI1_Block_B->CR1, SAI_xCR1_MCKDIV_Msk, div << SAI_xCR1_MCKDIV_Pos);
	MODIFY_REG(SAI2_Block_A->CR1, SAI_xCR1_MCKDIV_Msk, div << SAI_xCR1_MCKDIV_Pos);
	MODIFY_REG(SAI2_Block_B->CR1, SAI_xCR1_MCKDIV_Msk, div << SAI_xCR1_MCKDIV_Pos);
	MODIFY_REG(SAI3_Block_A->CR1, SAI_xCR1_MCKDIV_Msk, div << SAI_xCR1_MCKDIV_Pos);
	MODIFY_REG(SAI3_Block_B->CR1, SAI_xCR1_MCKDIV_Msk, div << SAI_xCR1_MCKDIV_Pos);
	MODIFY_REG(SAI4_Block_A->CR1, SAI_xCR1_MCKDIV_Msk, div << SAI_xCR1_MCKDIV_Pos);
	MODIFY_REG(SAI4_Block_B->CR1, SAI_xCR1_MCKDIV_Msk, div << SAI_xCR1_MCKDIV_Pos);
	if(sai_enabled) {sai_enable();}
}
