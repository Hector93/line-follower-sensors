/* 
 * File:                state_machine.c
 * Author:              Hector Manuel
 * Comments:
 * Revision history: 
 */


#ifndef STATE_MACHINE_H
#define	STATE_MACHINE_H


#include <xc.h> 
#include "protocol_registers.h"

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

 /**
 * @struct datagramStates
 *
 * @brief Represents the possible states of the state machine that processes a
 * a datagram this is updated by datagramStateMachineProcessByte.
 */   
typedef enum {
    STATE_SYNC = 0,
    STATE_ADDR,
    STATE_REGISTER,
    STATE_REGISTER_READ,
    STATE_REGISTER_WRITE,
    STATE_BYTE_0,
    STATE_BYTE_1,
    STATE_BYTE_2,
    STATE_BYTE_3,
    STATE_CRC
} datagramStates;

/**
 * @struct StateMachineStatus
 *
 * @brief Represents the possible states for when the state machine is sending
 * a datagram response.
 */   
typedef enum {
    SENDING = 0,
    OK,
    CONFLICT,
} StateMachineStatus;

/**
 * @struct delayRequest
 *
 * @brief this struct is used to configure code that will execute after a delay.
 * 
 * delay:  counter used to measure the elapsed time, once it reaches zero the
 *         function is executed if funPtr is not NULL and the flag is set to true
 *         if it's also not NULL.
 * flag:   a pointer to a boolean flag that is going to be set to True when the
 *         value of the delay variable is zero, if this pointer is NULL no update
 *         id performed.
 * funPtr: function pointer that's going to be called when the delay variable
 *         reaches zero, if this pointer is NULL nothing is called.
 */  
typedef struct{
    int8_t delay;
    volatile bool* flag;
    void (*funPtr)();
}delayRequest;


/**
 * @brief This function returns a pointer to the RX buffer used by the state
 * machine, the size of the array is configured with BUFFER_SIZE.
 * 
 * @return a volatile pointer to the Rx buffer
 * @see BUFFER_SIZE
 */
volatile char* getRxBuffer();

/**
 * @brief This function returns a pointer to the integer that holds the current
 * infrared sensor that's being sampled by the ADC.
 * 
 * @return a volatile pointer to the current IR sensor.
 */
volatile uint8_t* getActiveSensor();

/**
 * @brief returns the array containing the raw ADC values for the IR sensors,
 * the size of the array is configured by IR_SENSOR_COUNT
 * 
 * @return a volatile array of uint16_t type.
 * 
 * @see IR_SENSOR_COUNT
 */
volatile uint16_t* getRawADCValues();

/**
 * @brief returns the flag used to mark when all the IR sensors have been
 * sampled by the ADC.
 * 
 * @return a bool pointer to the sample complete flag.
 */
volatile bool* getSensorsSampleFlag();

/**
 * @brief returns a pointer to the delay requests array, this is used to store when a
 * flag or function needs to be executed. 
 * 
 * @return a volatile pointer of delayRequest.
 * 
 * @see delayRequest
 */
volatile delayRequest* getDelayRequests();

/**
 * @brief controls which IR sensor is going to be read and configures the shift
 * registers accordingly.
 * @param pos, receives the position of the sensor, it's responsibility of the
 * developer to send a value lower than IR_SENSOR_COUNT
 */
void setBits(uint8_t pos);

/**
 * @brief updates the IRSensor struct with the value passed, this includes the
 * update of the binary value.
 * 
 * @param[value] raw value retrieved by the ADC for a particular IR sensor.
 * @param[sensor] the desired sensor to update.
 * 
 * @see IRSensor
 * @see calibration of sensors TODO: document the calibration process
 */
void updateIRData(volatile uint16_t value, IRSensor* sensor);

/**
 * @brief initializes the state machine variables for a clean start
 * 
 * the delay requests are set to NULL to avoid calling garbage code,
 * the sensor array (IRSensor) is set to 0 values 
 * the shift registers are configured to 0 output, with this no IR sensor is
 * active.
 */
void initializeStateMachine();

/**
 * @brief updates the global state machine in 3 steps
 * 1. check if a datagram was received process it and creates a response if required
 * 2. if the elapsed time for a response has passed, then send the response,
 * this is configured by the TXDELAY in the conf.h file or in the config register
 * 3. updates the IR values when a sample is completed and if the interrupt is
 * enabled it sets the pin to high
 * 4. sets the next sample rate according to the SAMPLE_RATE configuration for
 *  measurements.
 */
void updateStateMachine();

/**
 * @brief this function blocks execution until a char has been sent via the UART
 * port.
 * 
 * this function is used when the USART can only send one byte at a time and
 * triggers an ISR each time.
 * 
 * @return state machine status, if the char was sent successfully then it'll
 * have the value OK.
 * 
 * @see StateMachineStatus
 */
StateMachineStatus waitSendConfirmation(void);

/**
 * @brief returns a pointer to the StateMachineStatus variable and with it the
 * state can be changed.
 * 
 * @return a volatile StateMachineStatus pointer since it can change in an ISR.
 */
volatile StateMachineStatus* getStateMachineStatus();

/**
 * @brief function to register operations to be executed later, it can be the
 * setting of a flag to true or a function call.
 * 
 * The developer needs to take in to account that the functions are going to be
 * executed in an ISR context, if it must be outside the ISR then use the flag.
 * 
 * 
 * @param[ms] the amount of milliseconds that need to pass to execute the function
 * or change the flag.
 * @param[flag] a pointer to the flag that is going to be changed, it can be NULL
 * if not used.
 * @param[funPtr] function pointer to the function that is going to be executed,
 * it can be NULL.
 * @param[delayRequests] array where the delay request is going to be stored.
 * @param[delayIndex] position in the array for the operation, at this moment
 * the positions of the array are reserved for specific operations.
 * 
 * @see delayRequest
 */
void ISRDelay(uint8_t ms, volatile bool* flag, void (*funPtr)(), volatile delayRequest* delayRequests, uint8_t delayIndex);

bool datagramStateMachineProcessByte(volatile uint8_t byte, volatile char* rxBuff);

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
bool processDatagram(volatile char* datagram, char* response, IRSensor* sensors);


#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* STATE_MACHINE_H */

