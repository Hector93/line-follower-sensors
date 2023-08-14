/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef HAL_FUNCTIONS_H
#define	HAL_FUNCTIONS_H

#include <xc.h> 


void shifRegisterInit();
void setData(bool bit);
void setClock(bool bit);
void setLatch(bool bit);
void setRst(bool bit);
volatile uint16_t ADCGetResult();
void ADCStartConversion();
void sendInt(bool state);
void ADCMUXInit();
void setChannel(uint8_t channel);
void ADCInit();
void timerInit();
void enableGlobalInt();
void waitTxReady(void);
void USART0_oneWireInit(void);
void USART0_oneWireSend(char* str, uint8_t size);
void USART0_setBaudrate(uint32_t baud);
void USART0_SetReceiveCompleteISR(bool val);



//TODO este include es necesario al igial que las variables de abajo?
//#include "config.h"

//extern volatile char charToSend;
//extern volatile status_t sendingStatus;
//extern volatile bool transmit;
//extern volatile uint8_t count;



#endif	/* HAL_FUNCTIONS_H */

