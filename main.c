 /*
 * MAIN Generated Driver File
 * 
 * @file main.c
 * 
 * @defgroup main MAIN
 * 
 * @brief This is the generated driver implementation file for the MAIN driver.
 *
 * @version MAIN Driver Version 1.0.0
*/

/*
� [2023] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/
#include "mcc_generated_files/system/system.h"
#include "mcc_generated_files/system/interrupt.h"


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stddef.h>
#include <stdbool.h>

#include "config.h"
#include "protocol_registers.h"
#include "hal_functions.h"
#include "state_machine.h"











volatile bool reply = false;

//datagram validation variables
//volatile bool received_datagram = false;
extern volatile bool received_datagram;











//volatile delayRequest delayRequests[MAX_DELAY_REQUESTS];


//void ISRDelay(uint8_t ms, volatile bool* flag, void (*funPtr)(), uint8_t delayIndex){
//    /*volatile delayRequest* slot = &delayRequests[delayIndex];
//    slot->delay = ms;
//    slot->flag = flag;
//    slot->funPtr = funPtr;
//    */
//    delayRequests[delayIndex].delay = ms;
//    delayRequests[delayIndex].flag = flag;
//    delayRequests[delayIndex].funPtr = funPtr;
//    //if(++delayIndex >= MAX_DELAY_REQUESTS){
//    //    delayIndex = 0;
//    //}
//    
//}







int main(void)
{   
    USART0_oneWireInit();
    shifRegisterInit();
    CLOCK_Initialize();
    ADCMUXInit();
    ADCInit();
    //TIMER configuration
    /* enable overflow interrupt */
    //TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
    /* set Normal mode */
    //TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
    /* disable event counting */
    //TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);
    /* set the period */
    //TCA0.SINGLE.PER = 300;  
    //TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc;
    PORTA.DIR |= ISR_PIN;    
    /* Enable global interrupts */
    //sei();
    timerInit();
    enableGlobalInt();
    
    IRSensor* sensors = getIrSensors();
    volatile uint16_t* rawADCValues = getRawADCValues();
    for(uint8_t i = 0; i < IR_SENSOR_COUNT; i++){
        sensors[i].lower = 0;
        sensors[i].upper = 0xFFFF;
        sensors[i].value = 0;
        sensors[i].procValue = false;
        rawADCValues[i] = 0;
    }
    volatile delayRequest* delayRequests = getDelayRequests();
    for(uint8_t i = 0; i < MAX_DELAY_REQUESTS; i++){
        delayRequests[i].delay = 0;
        delayRequests[i].flag = NULL;
        delayRequests[i].funPtr = NULL;
    }
    //USART0_oneWireSend("reading...\r\n", 12);
    
    
    USART0.CTRLA |= USART_RXCIE_bm;
    //ADC0.CTRLA |= ADC_ENABLE_bm;
    setRst(true);
    setBits(0);
    setChannel(0);
    //TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
    
    config_struct* cfgValues = getConfig();
    char* tx = getTxBuffer();
    volatile char* rx = getRxBuffer();
    //extern uint8_t sampleRate;//asd
    //extern uint8_t txDelay; //asd
    //static uint8_t acumulator = 0;
    //static uint8_t baudrate;
    //extern bool intEnable; //asd
    //static bool enable = false;   //asd
    //static bool enableCRC = true; //asd
    //ADCStartConversion();
    while (1) 
    {   
        if(received_datagram){
            received_datagram = false;
            if(processDatagram(rx, tx, sensors)){
                ISRDelay(cfgValues->txDelay,&reply, NULL, delayRequests, TXDELAY);
            }else{
                USART0.CTRLA |= USART_RXCIE_bm;
            }
        }
        if(reply == true){
            //sendInt(false);
            reply = false;
            USART0_oneWireSend((char*)tx, 8);
            USART0.CTRLA |= USART_RXCIE_bm;
        }
        
        volatile bool* sensorsSampleCmplt = getSensorsSampleFlag();
        if(*sensorsSampleCmplt && received_datagram == false){
            *sensorsSampleCmplt = false;
            
            for(uint8_t i = 0; i < IR_SENSOR_COUNT; i++){
                updateIRData(rawADCValues[i], &sensors[i]);
            }
            if(cfgValues->intEnable){
                sendInt(true);
            }
            if(cfgValues->sampleRate == 0){
                ADCStartConversion();
            }else{
            ISRDelay(cfgValues->sampleRate, NULL, ADCStartConversion, delayRequests, SAMPLE_RATE);
            }
            
        }
    }
}