// Author: sx107

#include <drv/i2c1.h>
#include <main.h>
#include <drv/clock.h>

volatile uint32_t _i2c1_error = 0;

#pragma GCC push_options
#pragma GCC optimize ("O0")

uint8_t i2c1_nack() {
	return I2C1->ISR & I2C_ISR_NACKF;
}

void i2c1_nack_clear() {
	I2C1->ICR |= I2C_ICR_NACKCF;
}

// I2C init
void i2c1_init() {
	_i2c1_error = 0;

	// I2C @ pins PC9, PA8
	RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN;
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODE6_Msk, 0b10 << GPIO_MODER_MODE6_Pos);
	MODIFY_REG(GPIOB->MODER, GPIO_MODER_MODE7_Msk, 0b10 << GPIO_MODER_MODE7_Pos);
	MODIFY_REG(GPIOB->OTYPER, GPIO_OTYPER_OT6_Msk, 0b1 << GPIO_OTYPER_OT6_Pos);
	MODIFY_REG(GPIOB->OTYPER, GPIO_OTYPER_OT7_Msk, 0b1 << GPIO_OTYPER_OT7_Pos);
	MODIFY_REG(GPIOB->OSPEEDR, GPIO_OSPEEDR_OSPEED6_Msk, 0b11 << GPIO_OSPEEDR_OSPEED6_Pos);
	MODIFY_REG(GPIOB->OSPEEDR, GPIO_OSPEEDR_OSPEED7_Msk, 0b11 << GPIO_OSPEEDR_OSPEED7_Pos);
	MODIFY_REG(GPIOB->AFR[0], GPIO_AFRL_AFSEL6_Msk, 4 << GPIO_AFRL_AFSEL6_Pos);
	MODIFY_REG(GPIOB->AFR[0], GPIO_AFRL_AFSEL7_Msk, 4 << GPIO_AFRL_AFSEL7_Pos);

	//NVIC_SetPriority(I2C1_ER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 1));
	//NVIC_EnableIRQ(I2C1_ER_IRQn);

	// Enable I2C clock
	RCC->APB1LENR |= RCC_APB1LENR_I2C1EN;
	RCC->APB1LRSTR |= RCC_APB1LRSTR_I2C1RST;
	_delayMs(1);
	RCC->APB1LRSTR &= ~RCC_APB1LRSTR_I2C1RST;
	I2C1->OAR2 &= ~I2C_OAR2_OA2EN; // Disable dual address
	// No global call, enable clock stretch, disable I2C, enable analog filter
	MODIFY_REG(I2C1->CR1, I2C_CR1_GCEN | I2C_CR1_NOSTRETCH | I2C_CR1_PE | I2C_CR1_ANFOFF, 0);
	I2C1->TIMINGR = 0x307076AD; // 0x00B03FD8 for fast mode (400 kHz)

	I2C1->OAR1 = 0;
	I2C1->CR1 |= I2C_CR1_PE;
	I2C1->CR2 &= ~I2C_CR2_NACK;

	I2C1->CR1 |= (I2C_CR1_NACKIE | I2C_CR1_ERRIE);
	for(uint8_t i = 0; i < 0xFE; i++) {}
}

void i2c1_deinit() {
	I2C1->ICR |= I2C_ICR_BERRCF | I2C_ICR_ARLOCF | I2C_ICR_OVRCF | I2C_ICR_PECCF | I2C_ICR_TIMOUTCF | I2C_ICR_NACKCF;
	I2C1->CR1 &= ~(I2C_CR1_NACKIE | I2C_CR1_ERRIE);
	I2C1->CR1 &= ~I2C_CR1_PE;
}

void i2c1_reset() {
	I2C1->CR2 |= I2C_CR2_STOP;
	(void) I2C1->ISR;

	I2C1->CR1 &= ~I2C_CR1_PE;
	WAITNBIT(I2C1->CR1, I2C_CR1_PE);
	I2C1->CR1 |= I2C_CR1_PE;
}

#pragma GCC pop_options

// I2C Error IRQ
void I2C1_ER_IRQHandler(void)
{
	_i2c1_error |= (I2C1->ISR) & (I2C_ISR_BERR | I2C_ISR_ARLO | I2C_ISR_OVR | I2C_ISR_TIMEOUT | I2C_ISR_PECERR);
	// SmBus errors are just ignored
	I2C1->ICR |= I2C_ICR_BERRCF | I2C_ICR_ARLOCF | I2C_ICR_OVRCF | I2C_ICR_PECCF | I2C_ICR_TIMOUTCF | I2C_ICR_NACKCF;
}

// Resets I2C, Sends stop condition, etc.
uint32_t i2c1_handle_error() {

	if(
		(_i2c1_error & I2C_ERR_AF) == I2C_ERR_AF || i2c1_nack()) {
		I2C1->CR2 |= I2C_CR2_STOP;
		WAITBIT(I2C1->CR2, I2C_CR2_STOP);
		(void) I2C1->ISR;
		_i2c1_error = 0;
		//_delayMs(1);
		return 0;
	}

	return _i2c1_error;
}

// Waits for a certain bit in I2C1->SR1 with a timeout
uint32_t i2c1_waitBit(uint32_t bit) {
	//_i2c1_error = 0;
	uint32_t timeOut = 0;
	while(((I2C1->ISR) & bit) != bit) {
		if(_i2c1_error != 0) {break;}
		if((timeOut++ > I2C1_TMO_VAL) && (I2C1_TMO_VAL != 0)) {
			_i2c1_error |= I2C_ERR_TMO;
			break;
		}
	}
	return _i2c1_error;
}

// Functions to give access to _i2c1_error from outside
void i2c1_reset_error() {
	_i2c1_error = 0;
}

uint32_t i2c1_get_error() {
	return _i2c1_error;
}

// Manual transmission functions
uint8_t i2c1_transmission_busy() {
	return ((I2C1->ISR) & I2C_ISR_BUSY) == I2C_ISR_BUSY ? 1 : 0;
}
uint32_t i2c1_transmission_start(uint8_t addr, _i2c_wrmode mode, uint16_t busy_timeout, uint8_t nbytes) {
	// Return if any error occurred earlier immediately, wait while I2C is busy
	if(_i2c1_error != 0) {return _i2c1_error;}
	uint16_t tmo_busy = busy_timeout;
	while((((I2C1->ISR) & I2C_ISR_BUSY) == I2C_ISR_BUSY) && (tmo_busy > 0)) {tmo_busy--;}
	if(tmo_busy == 0) {_i2c1_error |= I2C_ERR_BUSY;}

	if(_i2c1_error != 0) {return _i2c1_error;}
	// Enable ACK
	//MODIFY_REG(I2C1->CR1, I2C_CR1_POS | I2C_CR1_ACK, I2C_CR1_ACK);

	// Config the I2C
	MODIFY_REG(I2C1->CR2, I2C_CR2_NBYTES_Msk, (nbytes) << I2C_CR2_NBYTES_Pos);
	I2C1->CR2 |= I2C_CR2_AUTOEND;
	addr <<= 1; // Last bit is don't care
	MODIFY_REG(I2C1->CR2, I2C_CR2_SADD_Msk, addr << I2C_CR2_SADD_Pos);
	if(mode == I2C_READ) {I2C1->CR2 |= I2C_CR2_RD_WRN;}
	else {I2C1->CR2 &= ~I2C_CR2_RD_WRN;}


	// Send start condition
	I2C1->CR2 |= I2C_CR2_START;
	(void) I2C1->ISR;
	return _i2c1_error;
}
uint32_t i2c1_transmission_end(_i2c_wrmode mode) {
	//RETERR(i2c1_waitBit(I2C_ISR_TC), _i2c1_error);
	//I2C1->CR2 |= I2C_CR2_STOP;
	return i2c1_waitBit(I2C_ISR_STOPF);
}
uint32_t i2c1_transmission_write8(uint8_t byte) {
	i2c1_waitBit(I2C_ISR_TXIS);
	I2C1->TXDR = byte;
	return 0;
}
uint32_t i2c1_transmission_read8(volatile uint8_t* byte) {
	RETERR(i2c1_waitBit(I2C_ISR_RXNE), _i2c1_error);
	*byte = (I2C1->RXDR) & I2C_RXDR_RXDATA;
	return _i2c1_error;
}

void i2c1_transmission_ack_disable() {
	//MODIFY_REG(I2C1->CR1, I2C_CR1_POS | I2C_CR1_ACK, 0);
}

// Main write function
uint32_t i2c1_write_blocking(uint8_t addr, uint16_t sz, const volatile uint8_t* buf, uint16_t busy_timeout) {
	// Transmission start
	RETERR(i2c1_transmission_start(addr, I2C_WRITE, busy_timeout, sz), _i2c1_error);

	// Send data
	for(uint16_t i = 0; i < sz; i++) {
		RETERR(i2c1_transmission_write8(buf[i]), _i2c1_error);
	}

	return i2c1_transmission_end(I2C_WRITE);
}
// Main read function
uint32_t i2c1_read_blocking(uint8_t addr, uint16_t sz, volatile uint8_t* buf, uint16_t busy_timeout) {
	RETERR(i2c1_transmission_start(addr, I2C_READ, busy_timeout, sz), _i2c1_error);

	// Send data
	for(uint16_t i = 0; i < (sz); i++) {
		i2c1_transmission_read8(&(buf[i]));
	}

	// Stop
	RETERR(i2c1_transmission_end(I2C_READ), _i2c1_error);
	//buf[sz-1] = I2C1->RXDR & I2C_RXDR_RXDATA;
	return 0;
}


// Alternative read-write functions
uint32_t i2c1_write8(uint8_t addr, uint8_t byte) {
	return i2c1_write_blocking(addr, 1, &byte, 5000);
}

uint32_t i2c1_write16(uint8_t addr, uint16_t data) {
	uint8_t* d = (uint8_t*)(&data);
	return i2c1_write_blocking(addr, 2, d, 5000);
}

uint32_t i2c1_write32(uint8_t addr, uint32_t data) {
	uint8_t* d = (uint8_t*)(&data);
	return i2c1_write_blocking(addr, 4, d, 5000);
}

uint32_t i2c1_read8(uint8_t addr) {
	uint8_t retVal;
	i2c1_read_blocking(addr, 1, &retVal, 5000);
	return retVal;
}

uint32_t i2c1_read16(uint8_t addr) {
	uint16_t retVal;
	i2c1_read_blocking(addr, 2, (uint8_t*)&retVal, 5000);
	return retVal;
}

uint32_t i2c1_read32(uint8_t addr) {
	uint32_t retVal;
	i2c1_read_blocking(addr, 4, (uint8_t*)&retVal, 5000);
	return retVal;
}


// Register read-write

uint32_t i2c1_readRegister(uint8_t addr, uint8_t reg, uint16_t sz, uint8_t* buf) {
	RETERR(i2c1_write8(addr, reg), _i2c1_error);
	//for(uint8_t i = 0; i < 0xFF; i++) {}
	return i2c1_read_blocking(addr, sz, buf, 5000);
}

uint32_t i2c1_readRegister8(uint8_t addr, uint8_t reg) {
	RETERR(i2c1_write8(addr, reg), _i2c1_error);
	//for(uint8_t i = 0; i < 0xFF; i++) {}
	return i2c1_read8(addr);
}

uint32_t i2c1_writeRegister8(uint8_t addr, uint8_t reg, uint8_t val) {
	uint8_t sndBuf[] = {reg, val};
	return i2c1_write_blocking(addr, 2, sndBuf, 5000);
}
