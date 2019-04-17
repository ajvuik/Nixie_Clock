#include "SoftI2C.HPP"

SoftI2C::SoftI2C(uint8_t Sda_Pin, uint8_t Sck_Pin, uint8_t OwnAdress, uint8_t *RcvBuffer){
    this->_Sda_Pin=Sda_Pin;
    this->_Sck_Pin=Sck_Pin;
    this->_OwnAdress=OwnAdress;
    this->_RcvBuffer=RcvBuffer;
    SoftI2C::_Instances++;
    this->_CurInstance=SoftI2C::_Instances;
    instances[this->_CurInstance]=this;
}

SoftI2C::SoftI2C(uint8_t Sda_Pin, uint8_t Sck_Pin, uint8_t OwnAdress, uint8_t *RcvBuffer, bool IAmMaster){
    this->_Sda_Pin=Sda_Pin;
    this->_Sck_Pin=Sck_Pin;
    this->_OwnAdress=OwnAdress;
    this->_RcvBuffer=RcvBuffer;
    this->_IAmMaster=IAmMaster;
    SoftI2C::_Instances++;
    this->_CurInstance=SoftI2C::_Instances;
    instances[this->_CurInstance]=this;
}

SoftI2C::~SoftI2C(){
    SoftI2C::_Instances--;
}

void SoftI2C::begin(){
        attachInterrupt(digitalPinToInterrupt(this->_Sda_Pin), SoftI2C::instances[this->_CurInstance]->_InterruptHandler(), CHANGE);
        attachInterrupt(digitalPinToInterrupt(this->_Sck_Pin), SoftI2C::instances[this->_CurInstance]->_InterruptHandler(), CHANGE);
        this->_State=Idle;
}

void SoftI2C::loop(){
    switch (this->_State){
        case Idle:
            /* code */
            break;
    
        default:
            break;
    }
}