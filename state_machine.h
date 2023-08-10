/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef STATE_MACHINE_H
#define	STATE_MACHINE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "protocol_registers.h"
// TODO Insert appropriate #include <>

// TODO Insert C++ class definitions if appropriate

// TODO Insert declarations

// Comment a function and leverage automatic documentation with slash star star
/**
    <p><b>Function prototype:</b></p>
  
    <p><b>Summary:</b></p>

    <p><b>Description:</b></p>

    <p><b>Precondition:</b></p>

    <p><b>Parameters:</b></p>

    <p><b>Returns:</b></p>

    <p><b>Example:</b></p>
    <code>
 
    </code>

    <p><b>Remarks:</b></p>
 */
// TODO Insert declarations or function prototypes (right here) to leverage 
// live documentation

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 
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
} datagram_states;


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



typedef struct{
    int8_t delay;
    volatile bool* flag;
    void (*funPtr)();
}delayRequest;

void ISRDelay(uint8_t ms, volatile bool* flag, void (*funPtr)(), volatile delayRequest* delayRequests, uint8_t delayIndex);




char* getTxBuffer();
volatile char* getRxBuffer();
volatile delayRequest* getDelayRequests();
volatile uint8_t* getActiveSensor();
volatile bool* getSensorsSampleFlag();
IRSensor* getIrSensors();
volatile uint16_t* getRawADCValues();

void setBits(uint8_t pos);
void updateIRData(volatile uint16_t value, IRSensor* sensor);
#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* STATE_MACHINE_H */

