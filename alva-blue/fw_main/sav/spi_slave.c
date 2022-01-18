/*
 * SPI Slave driver for STM32 family processors
 *
 * 2010-2011 Michal Demin
 *
 */

#include "platform.h"

#include "gpio.h"

#include "spi_slave.h"

const struct gpio_init_table_t spi_slave_gpio[] = {
	{ // mosi
		.gpio = GPIOB, //GPIOA,
		.pin = GPIO_Pin_5, //7,
		.mode = GPIO_MODE_AF_PP, //IN_FLOATING,
		.speed = GPIO_SPEED_HIGH,
	},
	{ // PWMOUT
		.gpio = GPIOA,
		.pin = GPIO_Pin_8,
		.mode = GPIO_MODE_AF_PP,
		.speed = GPIO_SPEED_HIGH,
	},
	{ // MISO
		.gpio = GPIOB, //A,
		.pin = GPIO_Pin_4, //6,
		.mode = GPIO_MODE_AF_PP,
		.speed = GPIO_SPEED_HIGH,
/*		.mode = GPIO_MODE_OUT_PP,*/
		.state = GPIO_SET,
	},
	{ // clk
		.gpio = GPIOB, //A,
		.pin = GPIO_Pin_3, //5,
		.mode = GPIO_MODE_IN_FLOATING,
		.speed = GPIO_SPEED_HIGH,
	},
/*	{ // nss
		.gpio = GPIOA,
		.pin = GPIO_Pin_4,
		.mode = GPIO_MODE_IPD,
		.speed = GPIO_SPEED_HIGH,
	},*/
};

volatile uint8_t done = 1;
volatile uint8_t spi_dummy;

void spi_init(void)
{
	AFIO->MAPR |= 1;
	gpio_init(spi_slave_gpio, ARRAY_SIZE(spi_slave_gpio));

	// SPI module enable
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;

	// virtual NSS, bidir spi slave
	SPI1->CR1 |= SPI_CR1_SSI | SPI_CR1_SSM;// | SPI_CR1_BIDIOE | SPI_CR1_BIDIMODE;
/*	SPI1->CR2 |= SPI_CR2_SSOE;*/
	SPI1->CR2 &= ~(SPI_CR2_TXEIE | SPI_CR2_RXNEIE | SPI_CR2_ERRIE);
	SPI1->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

	SPI1->CR1 |= SPI_CR1_SPE;

	// Init DMA Channel (MEM->SPI)
	DMA1_Channel3->CPAR = (uint32_t)&SPI1->DR;
	DMA1_Channel3->CCR = (0x3 << 12) | (0 << 10) | (0 << 8) | DMA_CCR_MINC | DMA_CCR_TCIE;

	// Init DMA Channel (SPI->MEM)
	DMA1_Channel2->CPAR = (uint32_t)&SPI1->DR;
	DMA1_Channel2->CCR = (0x3 << 12) | (0 << 10) | (0 << 8) | DMA_CCR_DIR | DMA_CCR_TCIE;

	// enable interrupts in nvic
	NVIC_SetPriority(DMA1_Channel2_IRQn, 0);
	NVIC_EnableIRQ(DMA1_Channel2_IRQn);

	NVIC_SetPriority(DMA1_Channel3_IRQn, 0);
	NVIC_EnableIRQ(DMA1_Channel3_IRQn);

	NVIC_SetPriority(TIM1_UP_IRQn, 1);
	NVIC_EnableIRQ(TIM1_UP_IRQn);

	NVIC_SetPriority(SPI1_IRQn, 1);
	NVIC_EnableIRQ(SPI1_IRQn);

	// initialize timer 1us period
	uint32_t a = SystemFrequency / (1000 * 1000);
	TIM1->PSC = 1 - 1; // prescaler
	TIM1->ARR = a - 1; // period
	TIM1->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;

	// pwm enable
	TIM1->CCR1 = (a / 2) - 1;
	TIM1->CCMR1 |= (0x60 ); // oc1 to pwm1 mode, 
	TIM1->CCER |= (1); // polarity high, output enable

	TIM1->BDTR |= TIM_BDTR_MOE;
}

void spi_send(uint8_t *buf, uint32_t size)
{
	while (done == 0);

	done = 0;
//	TIM1->CR1 |= TIM_CR1_CEN;
	SPI1->CR1 &= ~SPI_CR1_SSI;

	SPI1->CR2 |= SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN;

	DMA1_Channel2->CMAR = (uint32_t)&spi_dummy;
	DMA1_Channel2->CNDTR = size;
	DMA1_Channel2->CCR |= DMA_CCR_EN;

	SPI1->DR = (uint8_t)0;

	if (size > 1) {
		DMA1_Channel3->CMAR = (uint32_t)&buf[0];
		DMA1_Channel3->CNDTR = size;
		DMA1_Channel3->CCR |= DMA_CCR_EN;
	}

	while (done == 0);
	//while ((SPI1->SR & SPI_SR_RXNE) == 0);
}

void SPI1_IRQHandler(void)
{
	SPI1->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
}

void DMA1_Channel2_IRQHandler(void)
{
	SPI1->CR2 &= ~SPI_CR2_RXDMAEN;

	DMA1->IFCR = DMA_IFCR_CGIF2;
	DMA1_Channel2->CCR &= ~DMA_CCR_EN;

	done = 1;
	SPI1->CR1 |= SPI_CR1_SSI;
	//TIM1->CR1 &= ~TIM_CR1_CEN;
}

void DMA1_Channel3_IRQHandler(void)
{
	SPI1->CR2 &= ~SPI_CR2_TXDMAEN;

	DMA1->IFCR = DMA_IFCR_CGIF3;
	DMA1_Channel3->CCR &= ~DMA_CCR_EN;

/*	done = 1;
	SPI1->CR1 |= SPI_CR1_SSI;*/
}

void TIM1_UP_IRQHandler(void)
{
	TIM1->SR &= ~TIM_SR_UIF;
	TIM1->DIER &= ~TIM_DIER_UIE;
//	TIM1->CR1 &= ~TIM_CR1_CEN;
}
