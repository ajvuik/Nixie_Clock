#include "SoftI2C.HPP"

SoftI2C::SoftI2C(uint8_t Sda, uint8_t Sck, uint8_t OwnAdress, uint8_t *RcvBuffer){
    this->_Sda=Sda;
    this->_Sck=Sck;
    this->_OwnAdress=OwnAdress;
    this->_RcvBuffer=RcvBuffer;
    SoftI2C::_Instances++;

}

SoftI2C::SoftI2C(uint8_t Sda, uint8_t Sck, uint8_t OwnAdress, uint8_t *RcvBuffer, bool IAmMaster){
    this->_Sda=Sda;
    this->_Sck=Sck;
    this->_OwnAdress=OwnAdress;
    this->_RcvBuffer=RcvBuffer;
    this->_IAmMaster=IAmMaster;
    SoftI2C::_Instances++;
}

void SoftI2C::begin(){
    
}