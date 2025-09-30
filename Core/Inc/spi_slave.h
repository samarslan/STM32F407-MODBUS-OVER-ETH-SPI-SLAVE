/*
 * spi_slave.h
 *
 *  Created on: Sep 30, 2025
 *      Author: Samet Arslan
 */

#ifndef INC_SPI_SLAVE_H_
#define INC_SPI_SLAVE_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* ----------------------------------------------------------------
 * Public API
 * ----------------------------------------------------------------*/

/**
 * @brief Initialize SPI slave logic and prepare for communication.
 *
 * Call this once after MX_SPI1_Init().
 */
void SPI_Slave_Init(void);

/**
 * @brief Polling-based SPI communication.
 *
 * Call periodically in main loop if not using interrupts.
 */
void SPI_Slave_Poll(void);

/**
 * @brief Manually set LED state (0=off, 1=on).
 *
 * @param index LED index (0=Green, 1=Orange, 2=Red, 3=Blue)
 * @param state Desired state
 */
void SPI_Slave_SetLED(uint8_t index, uint8_t state);

/**
 * @brief Get current LED state.
 *
 * @param index LED index (0=Green, 1=Orange, 2=Red, 3=Blue)
 * @return uint8_t 0=off, 1=on
 */
uint8_t SPI_Slave_GetLED(uint8_t index);

#endif /* INC_SPI_SLAVE_H_ */
