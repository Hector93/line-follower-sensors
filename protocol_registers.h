/* 
 * File:                protocol_registers.h
 * Author:              Hector Manuel
 * Comments:
 * Revision history: 
 */

 
#ifndef PROTOCOL_REGISTERS_H
#define	PROTOCOL_REGISTERS_H

/**
 * @file protocol_registers.h
 *
 * @brief Library for processing datagrams and managing infrared sensors.
 *
 * This header file contains functions for processing incoming datagrams and managing infrared sensors.
 */

#include <xc.h> 

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @struct IRSensor
 *
 * @brief Represents data from an infrared sensor.
 *
 * The IRSensor structure holds data related to an infrared sensor, including 
 * the sensor's current value, processing status, and upper and lower thresholds
 * for comparison.
 */
typedef struct {
    uint16_t value;  /**< The current value read from the infrared sensor. */
    bool procValue;  /**< A boolean flag indicating whether the sensor is 1 or 0. */
    uint16_t upper;  /**< The upper threshold value for the infrared sensor's data. */
    uint16_t lower;  /**< The lower threshold value for the infrared sensor's data. */
} IRSensor;

// TODO: validate all baudrates
/**
 * @struct baudrate_t
 *
 * @brief Represents the list of the possible baudrates
 *
 * The baudrate_t enum represents the possible list of the supported baudrates.
 */
typedef enum {
    BAUDRATE_9600   = 0,
    BAUDRATE_19200  = 1,
    BAUDRATE_28800  = 2,
    BAUDRATE_38400  = 3,
    BAUDRATE_57600  = 4,
    BAUDRATE_76800  = 5,
    BAUDRATE_115200 = 6,
    BAUDRATE_230400 = 7,
    BAUDRATE_460800 = 8
    
} baudrate_t;


typedef struct {
    uint8_t sampleRate: 7;
    bool intEnable;
    bool enable;
    uint8_t txDelay;
    bool enableCRC: 1;
    uint8_t acumulator: 6;
    baudrate_t baudrate; 
} config_struct;

/**
 * @struct block_t
 *
 * @brief Represents a block of infrared sensors and their position.
 *
 * The block_t enum represents the block sections for the infrared sensors
 * each block holds 3 sensors, since we have 16 sensors we use this to identify
 * where to position data of the ADC.
 */
typedef enum {
    BLOCK_0 = 0,  /**< block 0 contains the sensors [0-2]. */
    BLOCK_1 = 3,  /**< block 0 contains the sensors [3-5]. */
    BLOCK_2 = 6,  /**< block 0 contains the sensors [6-8]. */
    BLOCK_3 = 9,  /**< block 0 contains the sensors [9-11]. */
    BLOCK_4 = 12, /**< block 0 contains the sensors [12-14]. */
    BLOCK_5 = 15, /**< block 0 contains the sensors [15-16]. */
} block_t;

////

/**
 * @brief takes the first bit of the register address and reports if the operation is read or write.
 *
 * This function returns True if the operation requested by the master is a read
 * operation, it returns false if a write operation is requested.
 *
 * @param[reg] register address + r/w to check
 *
 * @return A boolean value indicating the if the operation is read operation or not.
 * @retval true read operation.
 * @retval false write operation.
 *
 */
inline bool isReadOperation(char reg){ return (reg & 0x01) ? false : true;}
    
/**
 * @brief This function processes or gets the data for the configuration register
 *
 * This function handles the data for the configuration register.
 * here the following options can be configured.
 * 
 * sampleRate: controls how fast the ADC would retrieve data for the 16 sensors 
 *              (7 bits).
 * intEnable:  controls if an interrupt request is sent to the master when 
 *             finishing the reading of the sensors (1 bit).
 * txDelay:    controls the time that the uc waits before sending an answer 
 *             to the master. (8 bits) TODO: document what the unit measures
 * enable:     controls if the uc starts reading data and sending interrupt
 *             requests (if enabled) to the master (1 bit).
 * enableCRC:  controls if the uc will validate the in or out data with the CRC
 *             if it's incorrect it'll ignore the datagram (1 bit).
 * acumulator: (6 bits)TODO: document what it does.
 * baudrate:   controls the baudrate of the communication by default it's 115200
 *             the supported baudrates are listed in the baudrate_t enum
 *             (8 bits)
 * 
 *
 * @param[reg] address of the requested operation.
 * @param[msg] pointer to an array containing the data used to configure the register.
 * @param[response] pointer to an array that it will contain the variables of the register if a read operation
 * was requested.
 * @param[IRSensor] unused, can be set to NULL.
 *
 * @return A boolean value indicating the success of processing the datagram.
 * @retval true The datagram was successfully processed and a response was generated.
 * @retval false the datagram was processed and no response is required.
 *
 * @see IRSensor
 * @see baudrate_t
 */
bool registerConfig(char reg, volatile char* msg, char* response, IRSensor* sensors);

/**
 * @brief Processes the datagram and generates a response based on the provided data.
 *
 * This function processes the incoming datagram, performs some calculations, and generates a response based on the provided data. It also interacts with the IRSensor structure to gather additional information.
 *
 * @param[in] datagram A pointer to the volatile character array containing the incoming datagram.
 * @param[out] response A pointer to the character array to store the generated response.
 * @param[in,out] sensors A pointer to the IRSensor structure containing data from infrared sensors.
 *
 * @return A boolean value indicating the success of processing the datagram.
 * @retval true The datagram was successfully processed and a response was generated.
 * @retval false An error occurred during processing.
 *
 * @warning The datagram and response buffers must be appropriately sized to avoid buffer overflows.
 * @note It is the caller's responsibility to manage memory for the response buffer.
 *
 * @see IRSensor
 */
bool registerStatus(char reg, volatile char* msg, char *response, IRSensor* sensors);

/**
 * @brief This function processes or gets the data for the raw infrared data register
 *
 * This function handles the retrieval of the data for the raw infrared values,
 * the infrared sensors are grouped in 6 blocks starting from block 0, each block
 * contains 3 sensors with the exception of block 5 which has only one sensor,
 * the size of each raw value is 10 bits, the values are in an array and left
 * aligned.
 * This register is read only since the data is obtained by the ADC.
 * 
 * rawValue0: 10 bits
 * rawValue1: 10 bits
 * rawValue2: 10 bits
 *
 * @param[reg] address of the requested operation.
 * @param[msg] pointer to an array containing the data used to configure the register.
 * @param[response] pointer to an array that it will contain the variables of the register if a read operation
 * was requested.
 * @param[block] index of the desired block of sensors.
 * @param[IRSensor] unused, can be set to NULL.
 *
 * @return A boolean value indicating the success of processing the datagram.
 * @retval true The datagram was successfully processed and a response was generated.
 * @retval false no response required since the register is read only.
 *
 * @see IRSensor
 * @see baudrate_t
 */
bool registerRawIRDataBlockX(char reg, volatile char* msg, char* response, block_t block, IRSensor* sensors);

/**
 * @brief This function processes or gets the data for the upper calibration infrared register
 * 
 * This function handles the configuration of the upper calibration values for
 * the infrared sensors, these values are used to compare when the digitized
 * value for the sensors, the infrared sensors are grouped in 6 blocks starting
 * from block 0, each block contains 3 sensors with the exception of block 5
 * which has only one sensor, the size of each raw value is 10 bits, the values
 * are in an array and left aligned.
 * 
 * upperCalib0: 10 bits
 * upperCalib1: 10 bits
 * upperCalib2: 10 bits
 *
 * @param[reg] address of the requested operation.
 * @param[msg] pointer to an array containing the data used to configure the register.
 * @param[response] pointer to an array that it will contain the variables of the register if a read operation
 * was requested.
 * @param[block] index of the desired block of sensors.
 * @param[IRSensor] unused, can be set to NULL.
 *
 * @return A boolean value indicating the success of processing the datagram.
 * @retval true The datagram was successfully processed and a response was generated.
 * @retval false no response required since the register is read only.
 *
 * @see IRSensor
 * @see baudrate_t
 */
bool registerUpperCalibblockX(char reg, volatile char* msg, char* response, block_t block, IRSensor* sensors);

/**
 * @brief This function processes or gets the data for the lower calibration infrared register
 *
 * This function handles the configuration of the lower calibration values for
 * the infrared sensors, these values are used to compare when the digitized
 * value for the sensors, the infrared sensors are grouped in 6 blocks starting
 * from block 0, each block contains 3 sensors with the exception of block 5
 * which has only one sensor, the size of each raw value is 10 bits, the values
 * are in an array and left aligned.
 * 
 * upperCalib0: 10 bits
 * upperCalib1: 10 bits
 * upperCalib2: 10 bits
 *
 * @param[reg] address of the requested operation.
 * @param[msg] pointer to an array containing the data used to configure the register.
 * @param[response] pointer to an array that it will contain the variables of the register if a read operation
 * was requested.
 * @param[block] index of the desired block of sensors.
 * @param[IRSensor] unused, can be set to NULL.
 *
 * @return A boolean value indicating the success of processing the datagram.
 * @retval true The datagram was successfully processed and a response was generated.
 * @retval false no response required since the register is read only.
 *
 * @see IRSensor
 * @see baudrate_t
 */
bool registerLowerCalibblockX(char reg, volatile char* msg, char* response, block_t block, IRSensor* sensors);

config_struct* getConfig();

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* PROTOCOL_REGISTERS_H */

