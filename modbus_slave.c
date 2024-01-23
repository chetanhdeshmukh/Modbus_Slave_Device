/**
 * @file uart_echo_interrupts_standby.c
 * @brief Modbus slave device functionality.
 * @details This code implements a Modbus slave device that listens for commands from
 * a master device and performs specific tasks as instructed.
 *
 * @author [Chetan]
 * @date 2024-01-11
 */

#include "ti_msp_dl_config.h"
#include <uart_driver.h>
#include <modbus_driver.h>

// Constants
#define MAXWAITCOUNT             1000       ///< Maximum wait loops for remaining packet data
#define ZERO_VALUE                  0       ///< Zero value constant
#define ONE_VALUE                   1       ///< One value constant
#define SEVEN_VALUE                 7       ///< Seven value constant



// Variables
uint8_t uiCheckIndex = 0;                       ///< Index for checking received data for slave ID
uint16_t uiWaitCount = 0;                       ///< Wait counter for remaining packet data
uint8_t uiTxLength;                             ///< Length of data to be transmitted


/**
 * @brief Main function for the Modbus slave device.
 * @details Initializes the system, sets up UART interrupts, and handles Modbus communication.
 */
int main(void)
{
  SYSCFG_DL_init();  // Initialize system configuration

  NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);   // Clear pending UART interrupt
  NVIC_EnableIRQ(UART_0_INST_INT_IRQN);         // Enable UART interrupt

    while (1)
    {

        // ________________________________ MODBUS _________________________________________

        // Handle errors and reset Modbus variables if an error is detected
        if (TRUE == bErrorFlag)
        {
            MODBUS_ResetState(&uiRxIndex,  &uiCheckIndex, &bErrorFlag, &uiWaitCount);
        }

        // Process Modbus packets if the received packet size is sufficient
        if (PACKET_SIZE <= uiRxIndex)
        {
            uiCheckIndex = ZERO_INDEX;
            while (uiCheckIndex < uiRxIndex)
            {
                if ((DEVICE_ID == uiRxBuff[uiCheckIndex]))
                {
                    if ((uiCheckIndex + SEVEN_VALUE) < uiRxIndex)
                    {
                        switch (uiRxBuff[uiCheckIndex + ONE_VALUE])
                        {
                        case READ_COMMAND:
                            // Verify checksum and prepare response for READ_COMMAND
                            if (MODBUS_VerifyChecksum(uiCheckIndex, uiRxBuff))
                            {
                                uiTxLength = MODBUS_PrepareResponse(uiCheckIndex, uiRxBuff, uiTxBuff);

                                UART_transmit(UART_0_INST, uiTxLength);

                                MODBUS_ResetState(&uiRxIndex,  &uiCheckIndex, &bErrorFlag, &uiWaitCount);
                                break;
                            }
                            else
                            {
                                uiCheckIndex++;
                            }
                            break;

                        default:
                            uiCheckIndex++;
                        }
                    }
                    else
                    {
                        //wait functionality : if incomplete packet received, system will wait for predefined iterations


                        if(uiWaitCount >= MAXWAITCOUNT){
                            MODBUS_ResetState(&uiRxIndex,  &uiCheckIndex, &bErrorFlag, &uiWaitCount);
                        }else
                        {
                            uiWaitCount++;
                        }


                        break;
                    }
                }
                else
                {
                    uiCheckIndex++;
                }
            }

            // Reset Modbus variables if no valid packet is found
            if (uiCheckIndex == uiRxIndex)
            {
                MODBUS_ResetState(&uiRxIndex,  &uiCheckIndex, &bErrorFlag, &uiWaitCount);
            }
        }

        // _____________________________ END OF MODBUS _______________________________________
    }
}


