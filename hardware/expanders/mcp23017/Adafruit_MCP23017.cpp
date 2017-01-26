/*************************************************** 
 This is a library for the MCP23017 i2c port expander

 These displays use I2C to communicate, 2 pins are required to
 interface
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <stdint.h>
#include "Adafruit_MCP23017.h"
#include <driver/gpio.h>
#include <driver/i2c.h>



// minihelper to keep Arduino backward compatibility
/*
static inline void wiresend(uint8_t x) {
	//Wire.write((uint8_t) x);
	//i2c_master_write(cmd, &x, 1, 0);
}
*/

/*
static inline uint8_t wirerecv(void) {

	//return Wire.read();
	return 0;
}
*/

static int bitRead(uint32_t x, uint8_t n) {
	return ((x & 1<<n) != 0);
}

static uint32_t bitWrite(uint32_t x, uint8_t n, uint8_t b) {
	return ((x & (~(1<< n))) | (b << n));
}

/**
 * Bit number associated to a give Pin
 */
uint8_t Adafruit_MCP23017::bitForPin(uint8_t pin){
	return pin%8;
}

/**
 * Register address, port dependent, for a given PIN
 */
uint8_t Adafruit_MCP23017::regForPin(uint8_t pin, uint8_t portAaddr, uint8_t portBaddr){
	return (pin<8)?portAaddr:portBaddr;
}

/**
 * Reads a given register
 */
uint8_t Adafruit_MCP23017::readRegister(uint8_t addr){
	// read the current GPINTEN
	//Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (MCP23017_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
	//wiresend(addr);
	i2c_master_write_byte(cmd, addr, 1);
	//Wire.endTransmission();
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 0);
	i2c_cmd_link_delete(cmd);
	//Wire.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (MCP23017_ADDRESS << 1) | I2C_MASTER_READ, 1 /* expect ack */);
	uint8_t tmpByte;
	i2c_master_read_byte(cmd, &tmpByte, 1);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 0);
	i2c_cmd_link_delete(cmd);
	return tmpByte;
}


/**
 * Writes a given register
 */
void Adafruit_MCP23017::writeRegister(uint8_t regAddr, uint8_t regValue){
	// Write the register
	//Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	//wiresend(regAddr);
	//wiresend(regValue);
	//Wire.endTransmission();
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (MCP23017_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
	i2c_master_write_byte(cmd, regAddr, 1);
	i2c_master_write_byte(cmd, regValue, 1);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 0);
	i2c_cmd_link_delete(cmd);
}


/**
 * Helper to update a single bit of an A/B register.
 * - Reads the current register value
 * - Writes the new register value
 */
void Adafruit_MCP23017::updateRegisterBit(uint8_t pin, uint8_t pValue, uint8_t portAaddr, uint8_t portBaddr) {
	uint8_t regValue;
	uint8_t regAddr=regForPin(pin,portAaddr,portBaddr);
	uint8_t bit=bitForPin(pin);
	regValue = readRegister(regAddr);

	// set the value for the particular bit
	regValue = bitWrite(regValue,bit,pValue);

	writeRegister(regAddr,regValue);
}

////////////////////////////////////////////////////////////////////////////////

/**
 * Initializes the MCP23017 given its HW selected address, see datasheet for Address selection.
 */
void Adafruit_MCP23017::begin(uint8_t addr) {
	if (addr > 7) {
		addr = 7;
	}
	i2caddr = addr;

	//Wire.begin();

	// set defaults!
	// all inputs on port A and B
	writeRegister(MCP23017_IODIRA,0xff);
	writeRegister(MCP23017_IODIRB,0xff);
}

/**
 * Initializes the default MCP23017, with 000 for the configurable part of the address
 */
void Adafruit_MCP23017::begin(void) {
	begin(0);
}

/**
 * Sets the pin mode to either INPUT or OUTPUT
 */
void Adafruit_MCP23017::pinMode(uint8_t p, uint8_t d) {
	updateRegisterBit(p,(d==GPIO_MODE_INPUT),MCP23017_IODIRA,MCP23017_IODIRB);
}

/**
 * Reads all 16 pins (port A and B) into a single 16 bits variable.
 */
uint16_t Adafruit_MCP23017::readGPIOAB() {
	uint16_t ba = 0;
	uint8_t a;

	// read the current GPIO output latches
	/*
	Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	wiresend(MCP23017_GPIOA);
	Wire.endTransmission();
	*/

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (MCP23017_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
	i2c_master_write_byte(cmd, MCP23017_GPIOA, 1);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 0);
	i2c_cmd_link_delete(cmd);

	/*
	Wire.requestFrom(MCP23017_ADDRESS | i2caddr, 2);

	a = wirerecv();
	ba = wirerecv();
	ba <<= 8;
	ba |= a;
	*/

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (MCP23017_ADDRESS << 1) | I2C_MASTER_READ, 1 /* expect ack */);
	i2c_master_read_byte(cmd, &a, 1);
	i2c_master_read_byte(cmd, (uint8_t *)&ba, 1);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 0);
	i2c_cmd_link_delete(cmd);

	ba <<= 8;
	ba |= a;

	return ba;
}

/**
 * Read a single port, A or B, and return its current 8 bit value.
 * Parameter b should be 0 for GPIOA, and 1 for GPIOB.
 */
uint8_t Adafruit_MCP23017::readGPIO(uint8_t b) {

	// read the current GPIO output latches
	/*
	Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	if (b == 0)
		wiresend(MCP23017_GPIOA);
	else {
		wiresend(MCP23017_GPIOB);
	}
	Wire.endTransmission();
	*/

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (MCP23017_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
	if (b == 0) {
		i2c_master_write_byte(cmd, MCP23017_GPIOA, 1);
	} else {
		i2c_master_write_byte(cmd, MCP23017_GPIOB, 1);
	}
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 0);
	i2c_cmd_link_delete(cmd);

	/*
	Wire.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
	return wirerecv();
	*/

	uint8_t byte;
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (MCP23017_ADDRESS << 1) | I2C_MASTER_READ, 1 /* expect ack */);
	i2c_master_read_byte(cmd, &byte, 1);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 0);
	i2c_cmd_link_delete(cmd);
	return byte;
}

/**
 * Writes all the pins in one go. This method is very useful if you are implementing a multiplexed matrix and want to get a decent refresh rate.
 */
void Adafruit_MCP23017::writeGPIOAB(uint16_t ba) {
	/*
	Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	wiresend(MCP23017_GPIOA);
	wiresend(ba & 0xFF);
	wiresend(ba >> 8);
	Wire.endTransmission();
	*/

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (MCP23017_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */);
	i2c_master_write_byte(cmd, MCP23017_GPIOA, 1);
	i2c_master_write_byte(cmd, ba & 0xFF, 1);
	i2c_master_write_byte(cmd, ba >> 8, 1);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 0);
	i2c_cmd_link_delete(cmd);
}

void Adafruit_MCP23017::digitalWrite(uint8_t pin, uint8_t d) {
	uint8_t gpio;
	uint8_t bit=bitForPin(pin);


	// read the current GPIO output latches
	uint8_t regAddr=regForPin(pin,MCP23017_OLATA,MCP23017_OLATB);
	gpio = readRegister(regAddr);

	// set the pin and direction
	gpio = bitWrite(gpio,bit,d);

	// write the new GPIO
	regAddr=regForPin(pin,MCP23017_GPIOA,MCP23017_GPIOB);
	writeRegister(regAddr,gpio);
}

void Adafruit_MCP23017::pullUp(uint8_t p, uint8_t d) {
	updateRegisterBit(p,d,MCP23017_GPPUA,MCP23017_GPPUB);
}

uint8_t Adafruit_MCP23017::digitalRead(uint8_t pin) {
	uint8_t bit=bitForPin(pin);
	uint8_t regAddr=regForPin(pin,MCP23017_GPIOA,MCP23017_GPIOB);
	return (readRegister(regAddr) >> bit) & 0x1;
}

/**
 * Configures the interrupt system. both port A and B are assigned the same configuration.
 * Mirroring will OR both INTA and INTB pins.
 * Opendrain will set the INT pin to value or open drain.
 * polarity will set LOW or HIGH on interrupt.
 * Default values after Power On Reset are: (false,flase, LOW)
 * If you are connecting the INTA/B pin to arduino 2/3, you should configure the interupt handling as FALLING with
 * the default configuration.
 */
void Adafruit_MCP23017::setupInterrupts(uint8_t mirroring, uint8_t openDrain, uint8_t polarity){
	// configure the port A
	uint8_t ioconfValue=readRegister(MCP23017_IOCONA);
	ioconfValue = bitWrite(ioconfValue,6,mirroring);
	ioconfValue = bitWrite(ioconfValue,2,openDrain);
	ioconfValue = bitWrite(ioconfValue,1,polarity);
	writeRegister(MCP23017_IOCONA,ioconfValue);

	// Configure the port B
	ioconfValue=readRegister(MCP23017_IOCONB);
	ioconfValue = bitWrite(ioconfValue,6,mirroring);
	ioconfValue = bitWrite(ioconfValue,2,openDrain);
	ioconfValue = bitWrite(ioconfValue,1,polarity);
	writeRegister(MCP23017_IOCONB,ioconfValue);
}

/**
 * Set's up a pin for interrupt. uses arduino MODEs: CHANGE, FALLING, RISING.
 *
 * Note that the interrupt condition finishes when you read the information about the port / value
 * that caused the interrupt or you read the port itself. Check the datasheet can be confusing.
 *
 */
void Adafruit_MCP23017::setupInterruptPin(uint8_t pin, uint8_t mode) {

	// set the pin interrupt control (0 means change, 1 means compare against given value);
//	updateRegisterBit(pin,(mode!=CHANGE),MCP23017_INTCONA,MCP23017_INTCONB);
	// if the mode is not CHANGE, we need to set up a default value, different value triggers interrupt

	// In a RISING interrupt the default value is 0, interrupt is triggered when the pin goes to 1.
	// In a FALLING interrupt the default value is 1, interrupt is triggered when pin goes to 0.
//	updateRegisterBit(pin,(mode==FALLING),MCP23017_DEFVALA,MCP23017_DEFVALB);

	// enable the pin for interrupt
//	updateRegisterBit(pin,HIGH,MCP23017_GPINTENA,MCP23017_GPINTENB);

}

uint8_t Adafruit_MCP23017::getLastInterruptPin(){
	uint8_t intf;

	// try port A
	intf=readRegister(MCP23017_INTFA);
	for(int i=0;i<8;i++) if (bitRead(intf,i)) return i;

	// try port B
	intf=readRegister(MCP23017_INTFB);
	for(int i=0;i<8;i++) if (bitRead(intf,i)) return i+8;

	return MCP23017_INT_ERR;

}
uint8_t Adafruit_MCP23017::getLastInterruptPinValue(){
	uint8_t intPin=getLastInterruptPin();
	if(intPin!=MCP23017_INT_ERR){
		uint8_t intcapreg=regForPin(intPin,MCP23017_INTCAPA,MCP23017_INTCAPB);
		uint8_t bit=bitForPin(intPin);
		return (readRegister(intcapreg)>>bit) & (0x01);
	}

	return MCP23017_INT_ERR;
}


