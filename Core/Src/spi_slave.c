/*
 * spi_slave.c
 *
 *  Created on: Sep 30, 2025
 *      Author: Samet Arslan
 */

/* spi_slave.c - Slave MCU (SPI Slave with LED control)
 *
 * This module implements the SPI slave that receives ASCII commands
 * and controls local LEDs accordingly
 */

#include "main.h"
#include "spi_slave.h"
#include <string.h>
#include <stdio.h>
#include "stm32f4xx_hal_spi.h"
/* External SPI handle - configure in CubeMX:
 * - Mode: Slave
 * - Data Size: 8 bits
 * - Clock: Up to 2 MHz
 * - CPOL: Low, CPHA: 1 Edge
 * - NSS: Hardware NSS Input
 */
extern SPI_HandleTypeDef hspi1; // Adjust to your SPI instance

/* LED GPIO definitions - adjust to your board */
#define LED_PORT        GPIOD
#define LED_GREEN_PIN   GPIO_PIN_12
#define LED_ORANGE_PIN  GPIO_PIN_13
#define LED_RED_PIN     GPIO_PIN_14
#define LED_BLUE_PIN    GPIO_PIN_15

/* Protocol buffers */
#define BUFFER_SIZE 32
static uint8_t rxBuffer[BUFFER_SIZE];
static uint8_t txBuffer[BUFFER_SIZE];
static volatile uint8_t spiReady = 0;

/* LED state tracking */
static uint8_t ledStates[4] = {0}; // Green, Orange, Red, Blue

/* ----------------------------------------------------------------
 * LED Control Functions
 * ----------------------------------------------------------------*/

static void LED_SetState(uint8_t index, uint8_t state) {
    if (index >= 4) return;

    GPIO_PinState pinState = state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    ledStates[index] = state ? 1 : 0;

    switch (index) {
        case 0: HAL_GPIO_WritePin(LED_PORT, LED_GREEN_PIN, pinState); break;
        case 1: HAL_GPIO_WritePin(LED_PORT, LED_ORANGE_PIN, pinState); break;
        case 2: HAL_GPIO_WritePin(LED_PORT, LED_RED_PIN, pinState); break;
        case 3: HAL_GPIO_WritePin(LED_PORT, LED_BLUE_PIN, pinState); break;
    }
}

static void LED_SetAll(uint8_t state) {
    for (uint8_t i = 0; i < 4; i++) {
        LED_SetState(i, state);
    }
}

/* ----------------------------------------------------------------
 * Command Parser
 * ----------------------------------------------------------------*/

static void ProcessCommand(const char *cmdRaw, char *response) {
    char cmd[BUFFER_SIZE];
    size_t i = 0;

    /* Copy until newline or buffer end */
    while (i < BUFFER_SIZE - 1 && cmdRaw[i] != '\n' && cmdRaw[i] != '\0') {
        cmd[i] = cmdRaw[i];
        i++;
    }
    cmd[i] = '\0';  // ensure null-terminated


    /* LED command: LED:X# */
    if (strncmp(cmd, "LED:", 4) == 0) {

        char color = cmd[4];
        char state = cmd[5];

        if (state != '0' && state != '1') return;
        uint8_t stateVal = (state == '1') ? 1 : 0;

        switch (color) {
            case 'G': LED_SetState(0, stateVal); strcpy(response, "OK\n"); break;
            case 'O': LED_SetState(1, stateVal); strcpy(response, "OK\n"); break;
            case 'R': LED_SetState(2, stateVal); strcpy(response, "OK\n"); break;
            case 'B': LED_SetState(3, stateVal); strcpy(response, "OK\n"); break;
            case 'A': LED_SetAll(stateVal);       strcpy(response, "OK\n"); break;
        }
    }
    /* GET command: GET:LED */
    else if (strncmp(cmd, "GET:LED", 7) == 0) {
        snprintf(response, BUFFER_SIZE, "STA:%d%d%d%d\n",
                 ledStates[0], ledStates[1], ledStates[2], ledStates[3]);
    }
}

/* ----------------------------------------------------------------
 * SPI Communication (Interrupt-driven)
 * ----------------------------------------------------------------*/

void SPI_Slave_Init(void) {
    /* Initialize all LEDs to OFF */
    LED_SetAll(0);

    /* Prepare for first SPI reception */
    memset(rxBuffer, 0, BUFFER_SIZE);
    memset(txBuffer, 0, BUFFER_SIZE);
    strcpy((char *)txBuffer, "RDY\n"); // Ready message

    /* Start SPI in interrupt mode */
    HAL_SPI_TransmitReceive_IT(&hspi1, txBuffer, rxBuffer, BUFFER_SIZE);
}

/* HAL SPI Transfer Complete Callback */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	if (hspi->Instance == hspi1.Instance) {
        /* Prepare clean response buffer */
        char response[BUFFER_SIZE] = {0};
        strcpy(response, "ERR\n");   // default in case command is invalid

        /* Process received command */
        ProcessCommand((char *)rxBuffer, response);

        /* Prepare response for next transaction */
        memset(txBuffer, 0, BUFFER_SIZE);
        strncpy((char *)txBuffer, response, BUFFER_SIZE - 1);

        /* Clear RX buffer */
        memset(rxBuffer, 0, BUFFER_SIZE);

        /* Ready for next transaction */
        HAL_SPI_TransmitReceive_IT(&hspi1, txBuffer, rxBuffer, BUFFER_SIZE);
    }
}

/* HAL SPI Error Callback */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == hspi1.Instance) {
        /* Restart SPI communication on error */
        memset(rxBuffer, 0, BUFFER_SIZE);
        memset(txBuffer, 0, BUFFER_SIZE);
        strcpy((char *)txBuffer, "ERR\n");
        HAL_SPI_TransmitReceive_IT(&hspi1, txBuffer, rxBuffer, BUFFER_SIZE);
    }
}

/* ----------------------------------------------------------------
 * Polling-based alternative (for simpler implementation)
 * Call this in your main loop if not using interrupts
 * ----------------------------------------------------------------*/

void SPI_Slave_Poll(void) {
    /* Check if CS is low (master starting transaction) */

    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET) {
        memset(rxBuffer, 0, BUFFER_SIZE);

        /* Perform SPI transaction */
        if (HAL_SPI_TransmitReceive(&hspi1, txBuffer, rxBuffer,
                                    BUFFER_SIZE, 100) == HAL_OK) {
            /* Process command and prepare response */
            char response[BUFFER_SIZE];
            ProcessCommand((char *)rxBuffer, response);

            memset(txBuffer, 0, BUFFER_SIZE);
            strcpy((char *)txBuffer, response);
        }
    }
}

/* ----------------------------------------------------------------
 * Direct LED control API (for testing)
 * ----------------------------------------------------------------*/

void SPI_Slave_SetLED(uint8_t index, uint8_t state) {
    LED_SetState(index, state);
}

uint8_t SPI_Slave_GetLED(uint8_t index) {
    if (index >= 4) return 0;
    return ledStates[index];
}

