/*
 * sai.h
 *
 *  Created on: Aug 8, 2024
 *      Author: Sx107
 */

#ifndef DRV_SAI_H_
#define DRV_SAI_H_

#include <main.h>

#define BUFFER_SIZE 1024

extern DMA_BUFFER int32_t _sai_outBuf[BUFFER_SIZE+200];
extern DMA_BUFFER int32_t _sai_inBuf[BUFFER_SIZE+200];
extern volatile uint32_t sai_wrpos;
extern volatile int32_t sai_laps_difference;
extern volatile float sai_feedback;
extern volatile uint32_t sai_buffer_shift;

void sai_init();
void sai_enable();
void sai_disable();

void sai_instance_dma_configure(SAI_TypeDef* sai);
void sai_instance_dma_stop(SAI_TypeDef* sai);

void sai_global_init(SAI_TypeDef* sai);
void sai_block_init(SAI_Block_TypeDef* block);
void sai_init_pins(SAI_TypeDef* sai);



#endif /* DRV_SAI_H_ */
