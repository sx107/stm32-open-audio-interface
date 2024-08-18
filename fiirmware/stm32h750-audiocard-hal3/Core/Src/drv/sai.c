/*
 * sai.c
 *
 *  Created on: Aug 8, 2024
 *      Author: Sx107
 */

#include <drv/sai.h>
#include <main.h>

DMA_BUFFER int32_t _sai_outBuf[BUFFER_SIZE+200];
DMA_BUFFER int32_t _sai_inBuf[BUFFER_SIZE+200];
volatile uint32_t sai_wrpos = 0;
volatile int32_t sai_laps_difference = 0;
volatile float sai_feedback = 0;
volatile uint32_t sai_buffer_shift = 64;

const uint32_t sine[] = {0, 98165, 196271, 294258, 392069, 489643, 586922, 683848, 780361, \
		876405, 971921, 1066851, 1161139, 1254727, 1347559, 1439580, 1530734, \
		1620965, 1710220, 1798445, 1885587, 1971593, 2056411, 2139990, \
		2222281, 2303233, 2382797, 2460926, 2537573, 2612691, 2686236, \
		2758162, 2828427, 2896988, 2963805, 3028835, 3092042, 3153386, \
		3212830, 3270339, 3325878, 3379414, 3430914, 3480348, 3527685, \
		3572897, 3615957, 3656839, 3695518, 3731971, 3766176, 3798113, \
		3827761, 3855104, 3880125, 3902809, 3923141, 3941111, 3956706, \
		3969918, 3980739, 3989162, 3995182, 3998795, 4000000, 3998795, \
		3995182, 3989162, 3980739, 3969918, 3956706, 3941111, 3923141, \
		3902809, 3880125, 3855104, 3827761, 3798113, 3766176, 3731971, \
		3695518, 3656839, 3615957, 3572897, 3527685, 3480348, 3430914, \
		3379414, 3325878, 3270339, 3212830, 3153386, 3092042, 3028835, \
		2963805, 2896988, 2828427, 2758162, 2686236, 2612691, 2537573, \
		2460926, 2382797, 2303233, 2222281, 2139990, 2056411, 1971593, \
		1885587, 1798445, 1710220, 1620965, 1530734, 1439580, 1347559, \
		1254727, 1161139, 1066851, 971921, 876405, 780361, 683848, 586922, \
		489643, 392069, 294258, 196271, 98165, 0, -98165, -196271, -294258, \
		-392069, -489643, -586922, -683848, -780361, -876405, -971921, \
		-1066851, -1161139, -1254727, -1347559, -1439580, -1530734, -1620965, \
		-1710220, -1798445, -1885587, -1971593, -2056411, -2139990, -2222281, \
		-2303233, -2382797, -2460926, -2537573, -2612691, -2686236, -2758162, \
		-2828427, -2896988, -2963805, -3028835, -3092042, -3153386, -3212830, \
		-3270339, -3325878, -3379414, -3430914, -3480348, -3527685, -3572897, \
		-3615957, -3656839, -3695518, -3731971, -3766176, -3798113, -3827761, \
		-3855104, -3880125, -3902809, -3923141, -3941111, -3956706, -3969918, \
		-3980739, -3989162, -3995182, -3998795, -4000000, -3998795, -3995182, \
		-3989162, -3980739, -3969918, -3956706, -3941111, -3923141, -3902809, \
		-3880125, -3855104, -3827761, -3798113, -3766176, -3731971, -3695518, \
		-3656839, -3615957, -3572897, -3527685, -3480348, -3430914, -3379414, \
		-3325878, -3270339, -3212830, -3153386, -3092042, -3028835, -2963805, \
		-2896988, -2828427, -2758162, -2686236, -2612691, -2537573, -2460926, \
		-2382797, -2303233, -2222281, -2139990, -2056411, -1971593, -1885587, \
		-1798445, -1710220, -1620965, -1530734, -1439580, -1347559, -1254727, \
		-1161139, -1066851, -971921, -876405, -780361, -683848, -586922, \
		-489643, -392069, -294258, -196271, -98165};
#include <string.h>

#define S24_TO_F (1.0 / 8388608.0)
#define F_TO_S24 8388608

void effect_process(volatile int32_t* input, volatile int32_t* output, uint16_t nSamples) {

	for(uint16_t i = 0; i < nSamples; i+=2) {
		float in_l = input[i] * S24_TO_F;
		float in_r = input[i+1] * S24_TO_F;

		float o_left = in_l;
		float o_right = in_r;

		output[i] = o_left * F_TO_S24;
		output[i+1] = o_right * F_TO_S24;
	}
}


void sai_init() {

	sai_init_pins(SAI1);
	sai_init_pins(SAI2);
	// sai_init_pins(SAI3);
	// sai_init_pins(SAI4);

	sai_disable();
	sai_global_init(SAI1); sai_block_init(SAI1_Block_A); sai_block_init(SAI1_Block_B);
	sai_global_init(SAI2); sai_block_init(SAI2_Block_A); sai_block_init(SAI2_Block_B);
//	sai_global_init(SAI3); sai_block_init(SAI3_Block_A); sai_block_init(SAI3_Block_B);
//	sai_global_init(SAI4); sai_block_init(SAI4_Block_A); sai_block_init(SAI4_Block_B);
}

void sai_global_init(SAI_TypeDef* sai) {
	if (sai == SAI1) {
		RCC->APB2ENR |= RCC_APB2ENR_SAI1EN;
		volatile uint32_t tmp = (RCC->APB2ENR);
		(void) tmp;
	} else if (sai == SAI2) {
		RCC->APB2ENR |= RCC_APB2ENR_SAI2EN;
		(void) (RCC->APB2ENR);
	} else if (sai == SAI3) {
		RCC->APB2ENR |= RCC_APB2ENR_SAI3EN;
		(void) (RCC->APB2ENR);
	} else {
		RCC->APB4ENR |= RCC_APB4ENR_SAI4EN;
		(void) (RCC->APB4ENR);
	}

	if(sai == SAI1) {
		sai->GCR |= 0b01 << SAI_GCR_SYNCOUT_Pos;
	} else {
		sai->GCR |= 0b00 << SAI_GCR_SYNCIN_Pos;
	}
}
void sai_block_init(SAI_Block_TypeDef* block) {
	// Syncen
	uint32_t sync_mode = 0;
	uint32_t tr_mode = 0;

	if(block == SAI1_Block_A) {
		// Async master
		sync_mode = 0b00 << SAI_xCR1_SYNCEN_Pos;
	} else if (block == SAI1_Block_B) {
		// Sync with other part
		sync_mode = 0b01 << SAI_xCR1_SYNCEN_Pos;
	} else {
		// Sync with SAI1A
		sync_mode = 0b10 << SAI_xCR1_SYNCEN_Pos;
	}

	// Mode
	if(block == SAI1_Block_A) {
		// Master transmit
		tr_mode = 0b00 << SAI_xCR1_MODE_Pos;

	} else if (block == SAI2_Block_A || block == SAI3_Block_A || block == SAI4_Block_A) {
		// Slave transmit
		tr_mode = 0b10 << SAI_xCR1_MODE_Pos;
	} else {
		// Slave receive
		tr_mode = 0b11 << SAI_xCR1_MODE_Pos;
	}

	block->CR1 &= ~(SAI_xCR1_MODE | SAI_xCR1_PRTCFG |  SAI_xCR1_DS | SAI_xCR1_LSBFIRST | SAI_xCR1_CKSTR | SAI_xCR1_SYNCEN | SAI_xCR1_MONO | SAI_xCR1_OUTDRIV  | SAI_xCR1_DMAEN |  SAI_xCR1_NODIV | SAI_xCR1_MCKDIV | SAI_xCR1_OSR | SAI_xCR1_MCKEN);
	block->CR1 |= tr_mode | sync_mode | 0b1 << SAI_xCR1_CKSTR_Pos | 0b00 << SAI_xCR1_PRTCFG_Pos | 0b111 << SAI_xCR1_DS_Pos | 0 << SAI_xCR1_LSBFIRST_Pos/* | SAI_xCR1_MCKEN*/;

	block->CR2 &= ~(SAI_xCR2_FTH | SAI_xCR2_FFLUSH | SAI_xCR2_COMP | SAI_xCR2_CPL);

	block->FRCR &= (~(SAI_xFRCR_FRL | SAI_xFRCR_FSALL | SAI_xFRCR_FSDEF | SAI_xFRCR_FSPOL | SAI_xFRCR_FSOFF));
	block->FRCR |= SAI_xFRCR_FSOFF;
	block->FRCR |= SAI_xFRCR_FSDEF;
	block->FRCR |= (32-1) << SAI_xFRCR_FSALL_Pos | (64-1) << SAI_xFRCR_FRL_Pos;

	block->SLOTR &= (~(SAI_xSLOTR_FBOFF | SAI_xSLOTR_SLOTSZ | SAI_xSLOTR_NBSLOT | SAI_xSLOTR_SLOTEN));
	block->SLOTR |= 0b10 << SAI_xSLOTR_SLOTSZ_Pos;
	block->SLOTR |= (2-1) << SAI_xSLOTR_NBSLOT_Pos;
	block->SLOTR |= 0xFFFF << SAI_xSLOTR_SLOTEN_Pos;
}
void sai_init_pins(SAI_TypeDef* sai) {
	if(sai == SAI1) {
		RCC->AHB4ENR |= RCC_AHB4ENR_GPIOEEN;
		// AF
		MODIFY_REG(GPIOE->MODER, GPIO_MODER_MODE2_Msk, 0b10 << GPIO_MODER_MODE2_Pos);
		MODIFY_REG(GPIOE->MODER, GPIO_MODER_MODE3_Msk, 0b10 << GPIO_MODER_MODE3_Pos);
		MODIFY_REG(GPIOE->MODER, GPIO_MODER_MODE4_Msk, 0b10 << GPIO_MODER_MODE4_Pos);
		MODIFY_REG(GPIOE->MODER, GPIO_MODER_MODE5_Msk, 0b10 << GPIO_MODER_MODE5_Pos);
		MODIFY_REG(GPIOE->MODER, GPIO_MODER_MODE6_Msk, 0b10 << GPIO_MODER_MODE6_Pos);
		// AF6
		GPIOE->AFR[0] |= 6 << GPIO_AFRL_AFSEL2_Pos;
		GPIOE->AFR[0] |= 6 << GPIO_AFRL_AFSEL3_Pos;
		GPIOE->AFR[0] |= 6 << GPIO_AFRL_AFSEL4_Pos;
		GPIOE->AFR[0] |= 6 << GPIO_AFRL_AFSEL5_Pos;
		GPIOE->AFR[0] |= 6 << GPIO_AFRL_AFSEL6_Pos;
	} else if (sai == SAI2) {
		RCC->AHB4ENR |= RCC_AHB4ENR_GPIODEN;
		RCC->AHB4ENR |= RCC_AHB4ENR_GPIOEEN;
		// AF
		GPIOD->MODER |= 0b10 << GPIO_MODER_MODE11_Pos;
		GPIOE->MODER |= 0b10 << GPIO_MODER_MODE11_Pos;
		// Max speed
		GPIOD->OSPEEDR |= 0b11 << GPIO_OSPEEDR_OSPEED11_Pos;
		GPIOE->OSPEEDR |= 0b11 << GPIO_OSPEEDR_OSPEED11_Pos;
		// AF10
		GPIOD->AFR[1] |= 10 << GPIO_AFRH_AFSEL11_Pos;
		GPIOE->AFR[1] |= 10 << GPIO_AFRH_AFSEL11_Pos;
	} else if (sai == SAI3) {
		RCC->AHB4ENR |= RCC_AHB4ENR_GPIODEN;
		// AF
		GPIOD->MODER |= 0b10 << GPIO_MODER_MODE1_Pos;
		GPIOD->MODER |= 0b10 << GPIO_MODER_MODE9_Pos;
		// Max speed
		GPIOD->OSPEEDR |= 0b11 << GPIO_OSPEEDR_OSPEED1_Pos;
		GPIOD->OSPEEDR |= 0b11 << GPIO_OSPEEDR_OSPEED9_Pos;
		// AF6
		GPIOD->AFR[0] |= 6 << GPIO_AFRL_AFSEL1_Pos;
		GPIOD->AFR[1] |= 6 << GPIO_AFRH_AFSEL9_Pos;
	} else if (sai == SAI4) {
		RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN;
		//RCC->AHB4ENR |= RCC_AHB4ENR_GPIOFEN;
		// AF
		GPIOC->MODER |= 0b10 << GPIO_MODER_MODE1_Pos;
		//GPIOF->MODER |= 0b10 << GPIO_MODER_MODE6_Pos;
		// Max speed
		GPIOC->OSPEEDR |= 0b11 << GPIO_OSPEEDR_OSPEED1_Pos;
		//GPIOF->OSPEEDR |= 0b11 << GPIO_OSPEEDR_OSPEED6_Pos;
		// AF6
		GPIOC->AFR[0] |= 8 << GPIO_AFRL_AFSEL1_Pos;
		//GPIOF->AFR[0] |= 6 << GPIO_AFRL_AFSEL6_Pos;
	}
}

void sai_enable() {
	SAI1_Block_A->CR1 |= SAI_xCR1_SAIEN;
	SAI1_Block_B->CR1 |= SAI_xCR1_SAIEN;
	//SAI2_Block_A->CR1 |= SAI_xCR1_SAIEN;
	//SAI2_Block_B->CR1 |= SAI_xCR1_SAIEN;
	//SAI3_Block_A->CR1 |= SAI_xCR1_SAIEN;
	//SAI3_Block_B->CR1 |= SAI_xCR1_SAIEN;
	//SAI4_Block_A->CR1 |= SAI_xCR1_SAIEN;
	//SAI4_Block_B->CR1 |= SAI_xCR1_SAIEN;
}
void sai_disable() {
	SAI1_Block_A->CR1 &= ~SAI_xCR1_SAIEN; WAITNBIT(SAI1_Block_A->CR1, SAI_xCR1_SAIEN);
	SAI1_Block_B->CR1 &= ~SAI_xCR1_SAIEN; WAITNBIT(SAI1_Block_B->CR1, SAI_xCR1_SAIEN);
	//SAI2_Block_A->CR1 &= ~SAI_xCR1_SAIEN;
	//SAI2_Block_B->CR1 &= ~SAI_xCR1_SAIEN;
	//SAI3_Block_A->CR1 &= ~SAI_xCR1_SAIEN;
	//SAI3_Block_B->CR1 &= ~SAI_xCR1_SAIEN;
	//SAI4_Block_A->CR1 &= ~SAI_xCR1_SAIEN;
	//SAI4_Block_B->CR1 &= ~SAI_xCR1_SAIEN;
}

void sai_instance_dma_configure(SAI_TypeDef* sai) {
	for(uint16_t i = 0; i < BUFFER_SIZE; i++) {
		_sai_inBuf[i] = 0;
		_sai_outBuf[i] = sine[i];
	}

	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

	NVIC_SetPriority(DMA2_Stream0_IRQn,  NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 3));
	NVIC_EnableIRQ(DMA2_Stream0_IRQn);

	//RXDR
	DMA2_Stream1->PAR = (uint32_t) &(SAI1_Block_B->DR);
	DMA2_Stream1->M0AR = (uint32_t) _sai_inBuf;
	DMA2_Stream1->M1AR = (uint32_t) _sai_inBuf;
	DMA2_Stream1->NDTR = BUFFER_SIZE;

	DMA2_Stream1->CR = 0;
	DMA2_Stream1->CR |= 0b01 << DMA_SxCR_MBURST_Pos;
	DMA2_Stream1->FCR |= DMA_SxFCR_DMDIS | 0b11 << DMA_SxFCR_FTH_Pos;
	//DMA2_Stream1->CR |= DMA_SxCR_HTIE | DMA_SxCR_TCIE;
	DMA2_Stream1->CR |= (0u << DMA_SxCR_DIR_Pos);   // Peripheral to memory
	DMA2_Stream1->CR |= DMA_SxCR_MINC;   // Memory increment mode
	DMA2_Stream1->CR |= (3u << DMA_SxCR_PL_Pos);   // Very High priority

	DMA2_Stream1->CR |= DMA_SxCR_CIRC; // Circular mode
	DMA2_Stream1->CR |= 0b10 << DMA_SxCR_MSIZE_Pos;
	DMA2_Stream1->CR |= 0b10 << DMA_SxCR_PSIZE_Pos;
	//DMA1_Stream1->CR |= DMA_SxCR_HTIE | DMA_SxCR_TCIE;

	DMAMUX1_Channel9->CCR = 88;
	DMA2->LIFCR |= DMA_LIFCR_CTCIF1 | DMA_LIFCR_CHTIF1 | DMA_LIFCR_CTEIF1 | DMA_LIFCR_CDMEIF1 | DMA_LIFCR_CFEIF1;

	// TXDR
	DMA2_Stream0->PAR = (uint32_t) &(SAI1_Block_A->DR);
	DMA2_Stream0->M0AR = (uint32_t) _sai_outBuf;
	DMA2_Stream0->M1AR = (uint32_t) _sai_outBuf;
	DMA2_Stream0->NDTR = BUFFER_SIZE;

	DMA2_Stream0->CR = 0;
	DMA2_Stream0->CR |= 0b01 << DMA_SxCR_MBURST_Pos;
	DMA2_Stream0->FCR |= DMA_SxFCR_DMDIS | 0b11 << DMA_SxFCR_FTH_Pos;
	DMA2_Stream0->CR |= DMA_SxCR_TCIE;
	DMA2_Stream0->CR |= (1u << DMA_SxCR_DIR_Pos);   // Memory to peripheral
	DMA2_Stream0->CR |= DMA_SxCR_MINC;   // Memory increment mode
	DMA2_Stream0->CR |= (3u << DMA_SxCR_PL_Pos);   // Very High priority

	DMA2_Stream0->CR |= DMA_SxCR_CIRC; // Circular mode
	DMA2_Stream0->CR |= 0b10 << DMA_SxCR_MSIZE_Pos;
	DMA2_Stream0->CR |= 0b10 << DMA_SxCR_PSIZE_Pos;

	DMAMUX1_Channel8->CCR = 87;

	DMA2_Stream0->NDTR = BUFFER_SIZE;
	DMA2_Stream1->NDTR = BUFFER_SIZE;

	DMA2->LIFCR |= DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0;

	__ISB();
	__DSB();

	DMA2_Stream0->CR |= DMA_SxCR_EN;
	DMA2_Stream1->CR |= DMA_SxCR_EN;

	SAI1_Block_A->CR1 |= SAI_xCR1_DMAEN;
	SAI1_Block_B->CR1 |= SAI_xCR1_DMAEN;
}

void sai_instance_dma_stop(SAI_TypeDef* sai) {
	DMA2_Stream0->CR &= ~DMA_SxCR_EN;
	DMA2_Stream1->CR &= ~DMA_SxCR_EN;
}


int32_t fb_history[100];
uint32_t fbh_pos;

volatile uint32_t critical_error = 0;

void DMA2_Stream0_IRQHandler() {
	static float sai_int = 0;
	static float sai_prev = 0;
	static uint8_t prev_div;

	uint8_t div = (SAI1_Block_A->CR1 & SAI_xCR1_MCKDIV) >> SAI_xCR1_MCKDIV_Pos;
	if(div == 0) {div = 1;}
	if(prev_div != div) {
		sai_int = 0;
		sai_prev = 0;
	}
	prev_div = div;

	if ((DMA2->LISR & DMA_LISR_TCIF0) == DMA_LISR_TCIF0) {
		sai_laps_difference += 1;

		int32_t v = sai_laps_difference * BUFFER_SIZE + sai_buffer_shift - sai_wrpos;

		float freq = 4*(1.0 / (float)div);
		float k = 13;
		sai_int += v; // For some way this works better
		sai_feedback = (k*v + 0.00003 * sai_int + 0.00022 * (v - sai_prev));
		sai_prev = v;

		fb_history[fbh_pos] = v;
		fbh_pos++;
		if(fbh_pos > 100) {fbh_pos = 0;}
	}

	DMA2->LIFCR |= DMA_LIFCR_CTCIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CFEIF0;
}

