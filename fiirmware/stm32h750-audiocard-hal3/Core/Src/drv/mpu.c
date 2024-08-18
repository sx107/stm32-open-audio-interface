/*
 * mpu.c
 *
 *  Created on: Feb 7, 2024
 *      Author: Sx107
 */

#include <drv/mpu.h>

void mpu_init() {
	__DMB();
	MPU->CTRL = 0;

	MPU->RNR = 0;
	MPU->RBAR = 0x30000000 & 0xFFFFFFE0U;

	// 4KB Size
	MPU->RASR = (11 << MPU_RASR_SIZE_Pos) | (0b00 << MPU_RASR_SRD_Pos) | (0b1 << MPU_RASR_S_Pos) | (0b0 << MPU_RASR_B_Pos) | (0b0 << MPU_RASR_C_Pos) | (0b001 << MPU_RASR_TEX_Pos) | (0b011 << MPU_RASR_AP_Pos) | (0b1 << MPU_RASR_XN_Pos);
	MPU->RNR = 0;
	MPU->RASR |= MPU_RASR_ENABLE_Msk;

	MPU->CTRL |= MPU_CTRL_ENABLE_Msk | MPU_CTRL_PRIVDEFENA_Msk;
}
