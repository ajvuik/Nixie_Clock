/*****************************************************
*
*
*
*
******************************************************/
#ifndef SOFTI2C_H
#define SOFTI2C_H

#include <Arduino.h>
#include <vector>

#define Idle            0
#define StartDetect     1
#define AdressedWrite   10
#define AdressedRead    20
#define MasterSend      30
#define MasterRead      40

class SoftI2C {
    public:
    SoftI2C(uint8_t Sda, uint8_t Sck, uint8_t OwnAdress, uint8_t *RcvBuffer);
    SoftI2C(uint8_t Sda, uint8_t Sck, uint8_t OwnAdress, uint8_t *RcvBuffer, bool IAmMaster);
    ~SoftI2C();
    void loop(void);
    void begin();
    bool IsIdle(void);
    void Send(uint8_t *SendBuffer, uint8_t SendAmount);

    private:
    static void _InterruptHandler();
    static uint16_t _Instances;
    uint8_t _Sda;
    uint8_t _Sck;
    uint8_t _State;
    uint8_t _OwnAdress;
    uint8_t _SendAmount;
    bool _IAmMaster;
    uint8_t *_SendBuffer;
    uint8_t *_RcvBuffer;
};

uint16_t SoftI2C::_Instances=0;

#endif //SOFTI2C_H