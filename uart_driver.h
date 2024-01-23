/*
 * uart_driver.h
 *
 *  Created on: 06-Jan-2024
 *      Author: Chetan
 */

#ifndef UART_DRIVER_H_
#define UART_DRIVER_H_
#define BUFFER_SIZE 100  ///< Size of RX and TX buffers
#define TRUE 1
#define FALSE 0

// Buffers
uint8_t uiRxBuff[BUFFER_SIZE];  ///< Buffer for received data
uint8_t uiTxBuff[BUFFER_SIZE];  ///< Buffer for data to be transmitted

uint8_t uiRxIndex = 0;  ///< Index for received data
uint8_t uiTxIndex;  ///< Index for data to be transmitted
uint8_t uiTxBuffLength;  ///< Length of data to be transmitted
bool bErrorFlag = FALSE;  ///< Error flag

//Function Definitions

void UART_transmitByte(UART_Regs *uart, uint8_t data);
void UART_transmit(UART_Regs *uart, uint8_t uiLength);
uint8_t UART_receiveByte(UART_Regs *uart);

//Function Declarations

/**
 * @brief Write a byte of data to the TX FIFO of the UART.
 *
 * @param uart Pointer to the UART registers.
 * @param data Data to be written to the TX FIFO.
 */
void UART_transmitByte(UART_Regs *uart, uint8_t data)
{
    uart->TXDATA = data;
}

/**
 * @brief Transmit multiple bytes of data over UART.
 *
 * This function sends a specified length of data from the global TX buffer over UART.
 *
 * @param uart Pointer to the UART registers.
 * @param uiLength Number of bytes to transmit.
 */
void UART_transmit(UART_Regs *uart, uint8_t uiLength)
{
    uiTxIndex = 0;
    uiTxBuffLength = uiLength;
    while (((uart->STAT & UART_STAT_BUSY_MASK) == UART_STAT_BUSY_SET));
    UART_transmitByte(UART_0_INST, uiTxBuff[uiTxIndex++]);

}

/**
 * @brief Read a byte of data from the RX FIFO of the UART.
 *
 * @param uart Pointer to the UART registers.
 * @return The received byte of data.
 */
uint8_t UART_receiveByte(UART_Regs *uart)
{
    return ((uint8_t)(uart->RXDATA & UART_RXDATA_DATA_MASK));
}

/**
 * @brief UART interrupt handler.
 *
 * This function handles UART interrupts, including receiving data, transmitting data,
 * and handling parity or overrun errors.
 */
void UART_0_INST_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt (UART_0_INST))
    {
    case DL_UART_MAIN_IIDX_RX:
        // Receive data and update the RX buffer
        if (uiRxIndex != BUFFER_SIZE)
        {
            uiRxBuff[uiRxIndex++] = UART_receiveByte(UART_0_INST);
        }
        else
        {
            bErrorFlag = TRUE;
        }
        break;

    case DL_UART_MAIN_IIDX_TX:

        if (uiTxIndex < uiTxBuffLength)
        {
            UART_transmitByte(UART_0_INST, uiTxBuff[uiTxIndex++]);
        }
        else
        {
            uiTxIndex = 0;
            uiTxBuffLength = 0;
        }
        break;

    case DL_UART_MAIN_IIDX_PARITY_ERROR:
        // Set error flag on parity error
        bErrorFlag = TRUE;
        break;

    case DL_UART_MAIN_IIDX_OVERRUN_ERROR:
        // Set error flag on overrun error
        bErrorFlag = TRUE;
        break;

    default:
        break;
    }
}

#endif /* UART_DRIVER_H_ */
