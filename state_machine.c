#include <stdbool.h>
#include <stddef.h>

#include "config.h"
#include "state_machine.h"
#include "protocol_registers.h"
#include "hal_functions.h"

volatile datagram_states datagram_state = STATE_SYNC;
volatile bool received_datagram = false;



bool datagramCalcCRC(volatile char* datagram, uint8_t datagramLength) {
int i,j;
char received_crc = datagram[datagramLength - 1];
volatile char* crc = datagram + (datagramLength-1); // CRC located in last byte of message UCHAR currentByte;
*crc = 0;
char currentByte;
for (i=0; i<(datagramLength-1); i++) {
    currentByte = datagram[i];
    for (j=0; j<8; j++) {
      if ((*crc >> 7) ^ (currentByte&0x01))
      {
        *crc = (*crc << 1) ^ 0x07;
      }
else
      {
        *crc = (*crc << 1);
}
      currentByte = currentByte >> 1;
    } // for CRC bit
  } // for message byte
return received_crc == *crc;
}



bool datagramStateMachineProcessByte(volatile uint8_t byte, volatile char* rxBuff){
    bool validCRC = false;
    static bool isReadDatagram = false;
    uint8_t datagramSize;
    
    switch(datagram_state){
        case STATE_SYNC:
            datagram_state = byte == LS_SYNC ? STATE_ADDR : STATE_SYNC;
            return byte == LS_SYNC;
        case STATE_ADDR:
            datagram_state = byte == LS_ADDR ? STATE_REGISTER : STATE_SYNC;
            return byte == LS_ADDR;
        case STATE_REGISTER:
            isReadDatagram = isReadOperation(byte);
            datagram_state = isReadDatagram ? STATE_CRC : STATE_BYTE_0;
            return true; // validation of the register will come latter
        case STATE_BYTE_0:
            datagram_state = STATE_BYTE_1;
            return true;
        case STATE_BYTE_1:
            datagram_state = STATE_BYTE_2;
            return true;
        case STATE_BYTE_2:
            datagram_state = STATE_BYTE_3;
            return true;
        case STATE_BYTE_3:
            datagram_state = STATE_CRC;
            return true;
        case STATE_CRC:
            datagramSize = isReadDatagram ? DATAGRAM_READ_SIZE : DATAGRAM_WRITE_SIZE;
            config_struct* config = getConfig();
            
            validCRC = datagramCalcCRC(rxBuff,datagramSize);
            if (validCRC || config->enableCRC == false){
                received_datagram = true;
                //TODO remove this from function
                USART0.CTRLA &= ~(USART_RXCIE_bm);
            }
            datagram_state = STATE_SYNC;
            return false;
        default:
            datagram_state = STATE_SYNC;
            return false;
    }
}


bool processDatagram(volatile char* datagram, char* response, IRSensor* sensors){
    if(datagram[0] != LS_SYNC && datagram[1] != LS_ADDR){
        return false;
    }
    
    char reg = datagram[2];
    response[0] = LS_SYNC;
    response[1] = LS_ADDR;
    response[2] = reg;
    bool result = false;
    
    switch((reg & 0xFE) >> 1){
        case LS_REGISTER_CONFIG:
            result = registerConfig(reg, &datagram[3], &response[3], NULL);
            break;
        case LS_REGISTER_STATUS:
            result = registerStatus(reg, &datagram[3], &response[3], NULL);
            break;
        case LS_REGISTER_RAW_DATA_0:
            result = registerRawIRDataBlockX(reg,NULL, &response[3], BLOCK_0, sensors);
            break;
        case LS_REGISTER_RAW_DATA_1:
            result = registerRawIRDataBlockX(reg,NULL, &response[3], BLOCK_1, sensors);
            break;
        case LS_REGISTER_RAW_DATA_2:
            result = registerRawIRDataBlockX(reg,NULL, &response[3], BLOCK_2, sensors);
            break;
        case LS_REGISTER_RAW_DATA_3:
            result = registerRawIRDataBlockX(reg,NULL, &response[3], BLOCK_3, sensors);
            break;
        case LS_REGISTER_RAW_DATA_4:
            result = registerRawIRDataBlockX(reg,NULL, &response[3], BLOCK_4, sensors);
            break;
        case LS_REGISTER_RAW_DATA_5:
            result = registerRawIRDataBlockX(reg,NULL, &response[3], BLOCK_5, sensors);
            break;
        case LS_REGISTER_UPPER_CALIBRATION_0:
            result = registerUpperCalibblockX(reg, &datagram[3], &response[3], BLOCK_0, sensors);
            break;
        case LS_REGISTER_UPPER_CALIBRATION_1:
            result = registerUpperCalibblockX(reg, &datagram[3], &response[3], BLOCK_1, sensors);
            break;
        case LS_REGISTER_UPPER_CALIBRATION_2:
            result = registerUpperCalibblockX(reg, &datagram[3], &response[3], BLOCK_2, sensors);
            break;
        case LS_REGISTER_UPPER_CALIBRATION_3:
            result = registerUpperCalibblockX(reg, &datagram[3], &response[3], BLOCK_3, sensors);
            break;
        case LS_REGISTER_UPPER_CALIBRATION_4:
            result = registerUpperCalibblockX(reg, &datagram[3], &response[3], BLOCK_4, sensors);
            break;
        case LS_REGISTER_UPPER_CALIBRATION_5:
            result = registerUpperCalibblockX(reg, &datagram[3], &response[3], BLOCK_5, sensors);
            break;
        case LS_REGISTER_LOWER_CALIBRATION_0:
            result = registerLowerCalibblockX(reg, &datagram[3], &response[3], BLOCK_0, sensors);
            break;
        case LS_REGISTER_LOWER_CALIBRATION_1:
            result = registerLowerCalibblockX(reg, &datagram[3], &response[3], BLOCK_1, sensors);
            break;
        case LS_REGISTER_LOWER_CALIBRATION_2:
            result = registerLowerCalibblockX(reg, &datagram[3], &response[3], BLOCK_2, sensors);
            break;
        case LS_REGISTER_LOWER_CALIBRATION_3:
            result = registerLowerCalibblockX(reg, &datagram[3], &response[3], BLOCK_3, sensors);
            break;
        case LS_REGISTER_LOWER_CALIBRATION_4:
            result = registerLowerCalibblockX(reg, &datagram[3], &response[3], BLOCK_4, sensors);
            break;
        case LS_REGISTER_LOWER_CALIBRATION_5:
            result = registerLowerCalibblockX(reg, &datagram[3], &response[3], BLOCK_5, sensors);
            break;
        default :
            result = false;
    }
    if(result){
        datagramCalcCRC(response,8);
    }
    return result;
}


char tx[BUFFER_SIZE];
volatile char rx[BUFFER_SIZE];
volatile delayRequest delayRequests[MAX_DELAY_REQUESTS];

volatile uint8_t activeSensor = 0;
volatile bool sensorsSampleCmplt = false;

//TODO: 2 more allocations are done to avoid validating if the requested register
// is block 5, due to it only having 1 channel
volatile uint16_t rawADCValues[IR_SENSOR_COUNT+2];
IRSensor sensors[IR_SENSOR_COUNT+2];

volatile uint8_t* getActiveSensor(){
    return &activeSensor;
}

volatile bool* getSensorsSampleFlag(){
    return &sensorsSampleCmplt;
}

char* getTxBuffer(){
    return tx;
}

volatile char* getRxBuffer(){
    return rx;
}

volatile uint16_t* getRawADCValues(){
    return rawADCValues;
}

IRSensor* getIrSensors(){
    return sensors;
}

volatile delayRequest* getDelayRequests(){
    return delayRequests;
}

void ISRDelay(uint8_t ms, volatile bool* flag, void (*funPtr)(), volatile delayRequest* delayRequests, uint8_t delayIndex){
    /*volatile delayRequest* slot = &delayRequests[delayIndex];
    slot->delay = ms;
    slot->flag = flag;
    slot->funPtr = funPtr;
    */
    delayRequests[delayIndex].delay = ms;
    delayRequests[delayIndex].flag = flag;
    delayRequests[delayIndex].funPtr = funPtr;
    //if(++delayIndex >= MAX_DELAY_REQUESTS){
    //    delayIndex = 0;
    //}
    
}

void setBits(uint8_t pos){
    setLatch(false);
    uint32_t value = 0x8000 >> pos;
    for(uint8_t i = 0; i < IR_SENSOR_COUNT; i++){
        setClock(false);
        setData((value >> i) & 1);
        setClock(true);
        setData(false);
    }
    
    setClock(false);
    setData(false);
    setLatch(true);
}

void updateIRData(volatile uint16_t value, IRSensor* sensor){
    sensor->value = value;
    if(sensor->value >= sensor->upper){
        sensor->procValue = true;
    }else if(sensor->value <= sensor->lower){
        sensor->procValue = false;
    }
}
