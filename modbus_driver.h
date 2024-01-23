/**
 * @file modbus_driver.h
 *
 * @brief This file contains the declarations of functions and variables used in the Modbus driver.
 *
 * This file contains the declarations of functions and variables used in the Modbus driver.
 * The Modbus driver is used to communicate with a slave device using the Modbus protocol.
 *
 * @author Chetan
 * @date 10-Jan-2024
 */

#ifndef MODBUS_DRIVER_H_
#define MODBUS_DRIVER_H_


#define DEVICE_ID       0x01   ///< ID of the slave device

#define BUFFER_SIZE     100    ///< Size of RX and TX Buffer

#define PACKET_SIZE     8      ///< Size of the packet to be received from the master in case of READ command

#define READ_COMMAND    0x03   ///< ID for READ_COMMAND Command

#define WRITE_COMMAND   0x04   ///< ID for WRITE_COMMAND Command

#define TRUE            1      ///< Boolean value for TRUE

#define FALSE           0      ///< Boolean value for FALSE

#define SHIFT_8_BITS    8      ///< Shift value for 8 bits

#define ZERO_INDEX      0      ///< Index of 0

#define ONE_INDEX       1      ///< Index of 1

#define TWO_INDEX       2      ///< Index of 2

#define SIX_INDEX       6      ///< Index of 6

#define SEVEN_INDEX     7      ///< Index of 7

#define ZERO_VALUE      0      ///< Value of 0

#define ONE_VALUE       1      ///< Value of 1

#define TWO_VALUE       2      ///< Value of 2

#define THREE_VALUE     3      ///< Value of 3

#define FOUR_VALUE      4      ///< Value of 4

#define FIVE_VALUE      5      ///< Value of 5

#define SIX_VALUE       6      ///< Value of 6

/**
 * @brief Modbus Register containing data about the slave device
 */
uint16_t uiModbusRegister[] = {0x1122, 0x3344, 0x5566, 0x7788, 0x9900, 0xAABB,
                               0x1234, 0x4565, 0x5548, 0x9969};


// Function Prototype Declaration
uint8_t MODBUS_PrepareResponse(uint8_t uiCheckIndex, uint8_t* uiRxBuff, uint8_t* uiTxBuff);
void MODBUS_ResetState(uint8_t* uiRxIndex, uint8_t* uiCheckIndex,
                       bool* bErrorFlag, uint16_t* uiWaitCount);
bool MODBUS_VerifyChecksum(uint8_t uiCheckIndex, uint8_t* uiRxBuff);
uint16_t MODBUS_GetCheckSum(unsigned char *buf, int len);




/**
 * @brief Prepare the response to be sent to the master
 *
 * This function prepares the response to be sent to the master. It takes the index of the byte where SLAVE_ID is matched,
 * the RX buffer, and the TX buffer as input parameters. It returns the number of bytes to be sent to the master.
 *
 * @param[in] uiCheckIndex Index of the byte where SLAVE_ID is matched
 * @param[in] uiRxBuff Pointer to the RX buffer
 * @param[out] uiTxBuff Pointer to the TX buffer
 *
 * @return Number of bytes to be sent to the master
 */

uint8_t MODBUS_PrepareResponse(uint8_t uiCheckIndex, uint8_t* uiRxBuff, uint8_t* uiTxBuff)
{
    volatile uint8_t uiTempIndex;
    uint16_t uiTxCheckSum;

    uiTxBuff[ZERO_INDEX] = uiRxBuff[uiCheckIndex];
    uiTxBuff[ONE_INDEX] = uiRxBuff[uiCheckIndex + ONE_VALUE];

    uint16_t uiStart = (((uint16_t)uiRxBuff[uiCheckIndex + TWO_VALUE]) << SHIFT_8_BITS) | uiRxBuff[uiCheckIndex + THREE_VALUE];
    uint16_t uiLength = (((uint16_t)uiRxBuff[uiCheckIndex + FOUR_VALUE]) << SHIFT_8_BITS) | uiRxBuff[uiCheckIndex + FIVE_VALUE];

    uiTxBuff[TWO_INDEX] = ((uint8_t)uiLength) * TWO_VALUE;
    uiTempIndex = THREE_VALUE;

    for (uint16_t count = uiStart; count < (uiLength + uiStart); count++)
    {
        uiTxBuff[uiTempIndex++] = (uint8_t) ( uiModbusRegister[count] >> SHIFT_8_BITS);
        uiTxBuff[uiTempIndex++] = (uint8_t) uiModbusRegister[count];
    }

    uiTxCheckSum = MODBUS_GetCheckSum(uiTxBuff, uiTempIndex);

    uiTxBuff[uiTempIndex++] = (uint8_t)  uiTxCheckSum;
    uiTxBuff[uiTempIndex++] = (uint8_t) (uiTxCheckSum >> SHIFT_8_BITS);

    return uiTempIndex;
}

/**
 * @brief Reset the state of the Modbus driver
 *
 * This function resets the state of the Modbus driver. It takes the pointers to the RX index, TX index,
 * checksum index, error flag, and wait count as input parameters.
 *
 * @param[out] uiRxIndex Pointer to the RX index
 * @param[out] uiTxIndex Pointer to the TX index
 * @param[out] uiCheckIndex Pointer to the checksum index
 * @param[out] bErrorFlag Pointer to the error flag
 * @param[out] uiWaitCount Pointer to the wait count
 * @returns void
 */
void MODBUS_ResetState(uint8_t* uiRxIndex,  uint8_t* uiCheckIndex,
                       bool* bErrorFlag, uint16_t* uiWaitCount)
{
    *uiRxIndex =  ZERO_INDEX;
    *uiCheckIndex = ZERO_INDEX;
    *bErrorFlag = FALSE;
    *uiWaitCount = ZERO_VALUE;
}

/**
 * @brief Verify the checksum of the received packet
 *
 * This function verifies the checksum of the received packet. It takes the index of the byte where SLAVE_ID is matched
 * and the RX buffer as input parameters. It returns a boolean value indicating whether the checksum is valid or not.
 *
 * @param[in] uiCheckIndex Index of the byte where SLAVE_ID is matched
 * @param[in] uiRxBuff Pointer to the RX buffer
 *
 * @return Boolean value indicating whether the checksum is valid or not
 */
bool MODBUS_VerifyChecksum(uint8_t uiCheckIndex, uint8_t* uiRxBuff)
{
   // return true;
    uint16_t uiMasterCheckSum;
    uint8_t uiRxPacketLength = SIX_VALUE;
    uiMasterCheckSum = uiRxBuff[uiCheckIndex + SEVEN_INDEX];
    uiMasterCheckSum <<= SHIFT_8_BITS;
    uiMasterCheckSum |= uiRxBuff[uiCheckIndex + SIX_INDEX];
    //uiMasterCheckSum = (((uint16_t) uiRxBuff[SEVEN_INDEX]) << SHIFT_8_BITS) | ((uint16_t) uiRxBuff[SIX_INDEX]);
    uint8_t uiRxPacket[uiRxPacketLength];
    for(uint8_t uiCount = uiCheckIndex, uiIndex = ZERO_VALUE; uiCount < (uiCheckIndex + SIX_VALUE ); uiCount++, uiIndex++)
    {
       uiRxPacket[uiIndex] = uiRxBuff[uiCount];
    }
    if(uiMasterCheckSum == MODBUS_GetCheckSum(uiRxPacket, uiRxPacketLength))
    {
        return true;
    }else
    {
        return false;
    }

}

/**
 * @brief Calculate the Modbus checksum of the given buffer
 *
 * This function calculates the Modbus checksum of the given buffer. It takes a pointer to the buffer and the length of the buffer as input parameters.
 * @note This part of code is copied from stackoverflow
 * @param[in] buf Pointer to the buffer
 * @param[in] len Length of the buffer
 *
 * @return Modbus checksum of the buffer
 */
uint16_t MODBUS_GetCheckSum(unsigned char *buf, int len)
{
    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < len; pos++)
    {
        crc ^= (uint16_t)buf[pos]; // XOR byte into least sig. byte of crc

        for (int i = 8; i != 0; i--)
        { // Loop over each bit
            if ((crc & 0x0001) != 0)
            { // If the LSB is set
                crc >>= 1; // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else // Else LSB is not set
                crc >>= 1; // Just shift right
        }
    }

    return crc;
}








#endif /* MODBUS_DRIVER_H_ */
