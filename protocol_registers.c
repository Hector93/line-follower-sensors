#include <stdbool.h>
#include "protocol_registers.h"
#include "hal_functions.h"
#include "config.h"


//TODO implement better default values
//uint8_t sampleRate = 10;//asd
//uint8_t txDelay = 5; //asd
//static uint8_t acumulator = 0;
//static uint8_t baudrate;
//bool intEnable = false; //asd
//static bool enable = false;   //asd
//static bool enableCRC = true; //asd

config_struct cfgValues = {
    .sampleRate = 10,
    .enable = false,
    .txDelay = 5,
    .enableCRC = true,
    .acumulator = 0,
    .baudrate = BAUDRATE_115200
};


inline uint32_t array2int(volatile char* str){
    return (uint32_t)str[0] | 
           (uint32_t)str[1] << 0x08 |
           (uint32_t)str[2] << 0x10 |
           (uint32_t)str[3] << 0x18;
}

uint16_t getDecodedBaudrate(baudrate_t baud){
    //TODO: odcument where the return values come from and move it to the hal abstraction
    switch(baud){
        case BAUDRATE_9600: return 8334;
        case BAUDRATE_19200: return 4167;
        case BAUDRATE_28800: return 2778;
        case BAUDRATE_38400: return 2084;
        case BAUDRATE_57600: return 1389;
        case BAUDRATE_76800: return 1042;
        case BAUDRATE_115200: return 695;
        case BAUDRATE_230400: return 348;
        case BAUDRATE_460800: return 174;
        default: return 0;
    }
}

bool registerConfig(char reg, volatile char* msg, char* response, IRSensor* sensors){
    //enable ISR, ISR config, resp delay, software reset, acumulation, sample rate, baudrate, enable crc, enable, 
    bool result = false;
    if(isReadOperation(reg)){
        response[0] = cfgValues.sampleRate | (cfgValues.intEnable << 7);
        response[1] = cfgValues.txDelay;
        response[2] = cfgValues.enable | (cfgValues.enableCRC << 1) | (cfgValues.acumulator << 2);
        response[3] = cfgValues.baudrate;
        result = true;
    }else{
        cfgValues.sampleRate = msg[0] & 0x7F;
        cfgValues.intEnable = (msg[0] & 0x80) >> 7;
        cfgValues.txDelay = msg[1];
        cfgValues.enable = msg[2] & 0x01;
        cfgValues.enableCRC = (msg[2] & 0x02) >> 1;
        cfgValues.acumulator = (msg[2] & 0x0C) >> 2;
        if(getDecodedBaudrate(msg[3]) != 0){
            cfgValues.baudrate = getDecodedBaudrate(msg[3]);
            USART0_setBaudrate(cfgValues.baudrate);
        }
        
        if(cfgValues.enable){
            ADCStartConversion();
        }
        result = false;
    }
    return result;
}



bool registerStatus(char reg, volatile char* msg, char *response, IRSensor* sensors){
    // binary results, rst, valid_msg

    if(isReadOperation(reg)){
        uint16_t binaryDataSensors = 0;
        for(uint8_t i = 0; i < IR_SENSOR_COUNT; i++){
            binaryDataSensors |= sensors[i].value << i;
        }
        response[0] = binaryDataSensors & 0x00FF >> 0x00;
        response[1] = binaryDataSensors & 0xFF00 >> 0x08;
        response[2] = 0;
        response[3] = 0;
        return true;
    }
    return false;
}


bool registerRawIRDataBlockX(char reg, volatile char* msg, char* response, block_t block, IRSensor* sensors){
    if(isReadOperation(reg)){
        uint32_t aux =
        ((uint32_t)(sensors[block].value & 0x3FF) << 0) |
        ((uint32_t)(sensors[block+1].value & 0x3FF) << 10) |
        ((uint32_t)(sensors[block+2].value & 0x3FF) << 20);
    
//        response[0] = aux & 0x000000FF >> 0x00;
//        response[1] = aux & 0x0000FF00 >> 0x08;
//        response[2] = aux & 0x00FF0000 >> 0x10;
//        response[3] = aux & 0xFF000000 >> 0x18;
        response[0] = (aux >> 0x00) & 0xFF;
        response[1] = (aux >> 0x08) & 0xFF;
        response[2] = (aux >> 0x10) & 0xFF;
        response[3] = (aux >> 0x18) & 0xFF;
        return true;
    }
    return false;
}


bool registerUpperCalibblockX(char reg, volatile char* msg, char* response, block_t block, IRSensor* sensors){
    if(isReadOperation(reg)){
    uint32_t aux =
    ((uint32_t)(sensors[block].upper & 0x3FF) << 0) |
    ((uint32_t)(sensors[block+1].upper & 0x3FF) << 10) |
    ((uint32_t)(sensors[block+2].upper & 0x3FF) << 20);
    
//    response[0] = aux & 0x000000FF >> 0x00;
//    response[1] = aux & 0x0000FF00 >> 0x08;
//    response[2] = aux & 0x00FF0000 >> 0x10;
//    response[3] = aux & 0xFF000000 >> 0x18;
        response[0] = (aux >> 0x00) & 0xFF;
        response[1] = (aux >> 0x08) & 0xFF;
        response[2] = (aux >> 0x10) & 0xFF;
        response[3] = (aux >> 0x18) & 0xFF;
        return true;
    }
    uint32_t values = array2int(msg);
    sensors[block].upper = values & 0x3FF >> 0;
    sensors[block+1].upper = values & ((uint32_t)0x3FF << 10) >> 10;
    sensors[block+2].upper = values & ((uint32_t)0x3FF << 20) >> 20;
    return true;
}


bool registerLowerCalibblockX(char reg, volatile char* msg, char* response, block_t block, IRSensor* sensors){
    if(isReadOperation(reg)){
        uint32_t aux =
        ((uint32_t)(sensors[block].lower & 0x3FF) << 0) |
        ((uint32_t)(sensors[block+1].lower & 0x3FF) << 10) |
        ((uint32_t)(sensors[block+2].lower & 0x3FF) << 20);
    
        
        response[0] = (aux >> 0x00) & 0xFF;
        response[1] = (aux >> 0x08) & 0xFF;
        response[2] = (aux >> 0x10) & 0xFF;
        response[3] = (aux >> 0x18) & 0xFF;
        return true;
    }
    uint32_t values = array2int(msg);
    sensors[block].lower = values & 0x3FF >> 0;
    sensors[block+1].lower = values & ((uint32_t)0x3FF << 10) >> 10;
    sensors[block+2].lower = values & ((uint32_t)0x3FF << 20) >> 20;
    return true;
}

config_struct* getConfig(){
    return &cfgValues;
}