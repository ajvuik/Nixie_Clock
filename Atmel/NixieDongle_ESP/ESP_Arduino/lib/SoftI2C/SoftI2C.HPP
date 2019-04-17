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

#ifndef I2C_Channels
#define I2C_Channels    1
#endif

class SoftI2C {
    public:
    SoftI2C(uint8_t Sda_Pin, uint8_t Sck_Pin, uint8_t OwnAdress, uint8_t *RcvBuffer);
    SoftI2C(uint8_t Sda_Pin, uint8_t Sck_Pin, uint8_t OwnAdress, uint8_t *RcvBuffer, bool IAmMaster);
    ~SoftI2C();
    void begin();
    void loop(void);
    bool IsIdle(void);
    void Send(uint8_t *SendBuffer, uint8_t SendAmount);

    private:
    static SoftI2C * instances[I2C_Channels];
    static uint16_t _Instances;
    void _InterruptHandler();
    uint16_t _CurInstance;
    uint8_t _Sda_Pin;
    uint8_t _Sck_Pin;
    uint8_t _State;
    bool _Start;
    bool _Stop;
    uint8_t _OwnAdress;
    uint8_t _SendAmount;
    bool _IAmMaster;
    uint8_t *_SendBuffer;
    uint8_t *_RcvBuffer;
};

SoftI2C * SoftI2C::instances[I2C_Channels]=NULL;
uint16_t SoftI2C::_Instances=0;

#endif //SOFTI2C_H