#include "platform.h"

#include "gpio.h"
#include "bsp_i2c.h"

#include "i2c.h"


void i2c_init(uint32_t speed)
{
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

	gpio_init(i2c_gpio, i2c_gpio_cnt);

	I2C1->CR1 &= ~I2C_CR1_PE;

	uint32_t freq = SystemFrequency_APB1Clk / 1000000;
	I2C1->CR2 = freq;

	uint32_t duty = 2;

	uint32_t ccr = 0;
	if (speed <= 100000) {
		uint32_t r = SystemFrequency_APB1Clk / (speed << 1);
		if (r < 0x04) {
			r = 0x04;
		}
		ccr |= r;
		I2C1->TRISE = freq+1;
	}/* else {
		uint32_t r;
		if (duty == 2) {
			r = SystemFrequency_APB1Clk / (speed * 3);
		} else { // 16/9
			r = SystemFrequency_APB1Clk / (speed * 25);
		}
		// TODO: finish
	}*/

	I2C1->CCR = ccr;
	I2C1->CR1 |= I2C_CR1_ACK;
	I2C1->OAR1 = 0x4000 | 0x15; // own address;
	
	I2C1->CR1 |= I2C_CR1_PE;
}

void i2c_start(uint8_t tx,  uint8_t slaveAddress)
{
	tx = !!tx;

	while(I2C1->SR2 & I2C_SR2_BUSY);

	I2C1->CR1 |= I2C_CR1_START;
	while(!((I2C1->SR1 & I2C_SR1_SB) && (I2C1->SR2 & (I2C_SR2_BUSY | I2C_SR2_MSL))));

	I2C1->DR = slaveAddress << 1 | (!tx);

	if (tx) {
		while(!((I2C1->SR1 & (I2C_SR1_SB | I2C_SR1_ADDR | I2C_SR1_TXE)) && (I2C1->SR2 & (I2C_SR2_BUSY | I2C_SR2_MSL | I2C_SR2_TRA))));
	} else {
		while(!((I2C1->SR1 & (I2C_SR1_SB | I2C_SR1_ADDR)) && (I2C1->SR2 & (I2C_SR2_BUSY | I2C_SR2_MSL))));
	}
}

void i2c_stop(void)
{
	I2C1->CR1 |= I2C_CR1_STOP;
}

void i2c_write_byte(uint8_t data)
{
	I2C1->DR = data;
	while(!((I2C1->SR1 & (I2C_SR1_BTF | I2C_SR1_TXE)) && (I2C1->SR2 & (I2C_SR2_BUSY | I2C_SR2_MSL | I2C_SR2_TRA))));
}

uint8_t i2c_read_byte(void)
{
	uint8_t data;

	while(!((I2C1->SR1 & (I2C_SR1_RXNE)) && (I2C1->SR2 & (I2C_SR2_BUSY))));
	data = I2C1->DR;

	return data;
}

