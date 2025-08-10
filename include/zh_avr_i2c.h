#pragma once

#include "FreeRTOS.h"
#include "event_groups.h"
#include "avr/io.h"
#include "avr/interrupt.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum
	{
		AVR_FAIL = -1,
		AVR_OK,
		AVR_ERR_NO_MEM,
		AVR_ERR_INVALID_ARG,
		AVR_ERR_INVALID_STATE,
		AVR_ERR_INVALID_SIZE,
		AVR_ERR_NOT_FOUND,
		AVR_ERR_NOT_SUPPORTED,
		AVR_ERR_TIMEOUT,
		AVR_ERR_INVALID_RESPONSE,
		AVR_ERR_INVALID_CRC,
		AVR_ERR_INVALID_VERSION,
		AVR_ERR_NOT_FINISHED,
		AVR_ERR_NOT_ALLOWED
	} avr_err_t;

	/**
	 * @brief Initialize I2C bus.
	 *
	 * @param[in] pullup Using internal pullup resistors.
	 *
	 * @return AVR_OK if success or an error code otherwise.
	 */
	avr_err_t zh_avr_i2c_master_init(const bool pullup);

	/**
	 * @brief Probe I2C address.
	 *
	 * @param[in] addr Address I2C device.
	 * @param[in] xTicksToWait Wait timeout in FreeRTOS ticks.
	 *
	 * @return AVR_OK if success or an error code otherwise.
	 */
	avr_err_t zh_avr_i2c_master_probe(const uint8_t addr, TickType_t xTicksToWait);

	/**
	 * @brief Send data to I2C address.
	 *
	 * @param[in] addr Address I2C device.
	 * @param[in] data Pointer to data for send.
	 * @param[in] size Data size.
	 * @param[in] xTicksToWait Wait timeout in FreeRTOS ticks.
	 *
	 * @return AVR_OK if success or an error code otherwise.
	 */
	avr_err_t zh_avr_i2c_master_transmit(const uint8_t addr, uint8_t *data, uint8_t size, TickType_t xTicksToWait);

	/**
	 * @brief Read data from I2C address.
	 *
	 * @param[in] addr Address I2C device.
	 * @param[out] data Pointer to buffer for read.
	 * @param[in] size Data size.
	 * @param[in] xTicksToWait Wait timeout in FreeRTOS ticks.
	 *
	 * @return AVR_OK if success or an error code otherwise.
	 */
	avr_err_t zh_avr_i2c_master_receive(const uint8_t addr, uint8_t *data, uint8_t size, TickType_t xTicksToWait);

	avr_err_t zh_avr_i2c_master_transmit_register(const uint8_t addr, uint16_t *reg, uint8_t *data, uint8_t size, TickType_t xTicksToWait); // To Do
	avr_err_t zh_avr_i2c_master_receive_register(const uint8_t addr, uint16_t *reg, uint8_t *data, uint8_t size, TickType_t xTicksToWait);	// To Do

#ifdef __cplusplus
}
#endif