/*
 * i2c_common.h
 *
 *  Created on: 10 дек. 2022 г.
 *      Author: Sx107
 */

#ifndef I2C_COMMON_H_
#define I2C_COMMON_H_

#define I2C_ERR_BERR 	I2C_ISR_BERR
#define I2C_ERR_ARLO 	I2C_ISR_ARLO
#define I2C_ERR_AF 		I2C_ISR_NACKF
#define I2C_ERR_OVR 	I2C_ISR_OVR
#define I2C_ERR_PEC 	I2C_ISR_PECERR
#define I2C_ERR_TMOSYS	I2C_ISR_TIMEOUT
#define I2C_ERR_TMO		(0b1 << 29) // This is NOT bit 14 SMBus timeout error!
#define I2C_ERR_BUSY 	(0b1 << 30) // I2C is busy.
#define I2C_ERR_OTHER	(0b1 << 31) // Other

typedef enum {I2C_WRITE, I2C_READ} _i2c_wrmode;

#endif /* I2C_COMMON_H_ */
