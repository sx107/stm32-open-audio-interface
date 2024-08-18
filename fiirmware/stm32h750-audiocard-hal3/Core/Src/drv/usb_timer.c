/*
 * usb_timer.c
 *
 *  Created on: Aug 13, 2024
 *      Author: Sx107
 */

#include <drv/usb_timer.h>
#include <main.h>

uint32_t _last_usbtim_val = 0x60000;
uint8_t _usbtim_cnt;
uint32_t _usbtim_val;

void usb_timer_init() {
	RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN;
	MODIFY_REG(GPIOA->MODER, GPIO_MODER_MODE4_Msk, 0b10 << GPIO_MODER_MODE4_Pos);
	GPIOA->AFR[0] |= 2 << GPIO_AFRL_AFSEL4_Pos;

	NVIC_SetPriority(TIM5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 2));
	NVIC_EnableIRQ(TIM5_IRQn);

	RCC->APB1LENR |= RCC_APB1LENR_TIM5EN;
	TIM5->PSC = 0;
	TIM5->ARR = 0xFFFFFFFF;
	TIM5->CR1 |= TIM_CR1_ARPE;
	TIM5->DIER |= TIM_DIER_TIE;

	TIM5->SMCR |= TIM_SMCR_ECE;

	TIM5->SMCR |= 0b011 << TIM_SMCR_TS_Pos | 0b01 << 20;
	TIM5->SMCR |= 0b100 << TIM_SMCR_SMS_Pos;


	TIM5->CCMR1 |= 0b11 << TIM_CCMR1_CC1S_Pos;
	TIM5->CCER |= TIM_CCER_CC1E;


	__DMB();
	TIM5->CR1 |= TIM_CR1_CEN;
	TIM5->EGR |= TIM_EGR_UG;
}

void TIM5_IRQHandler() {


	uint32_t tmp = TIM5->CCR1;
	//_last_usbtim_val = tmp << 6;
	_usbtim_val += tmp;
	_usbtim_cnt++;
	if(_usbtim_cnt >= 8) {
		_last_usbtim_val = _usbtim_val << 3;
		_usbtim_val = 0;
		_usbtim_cnt = 0;
	}
	TIM5->SR = 0;
}

uint32_t usb_fb_value() {
	return _last_usbtim_val;
}
