/*
 * SPI Slave driver for STM32 family processors
 *
 * 2010-2011 Michal Demin
 *
 */

#include "platform.h"

#include "gpio.h"

#include "spi_slave.h"


volatile uint8_t done = 1;

uint16_t send_buf[4*256];

uint8_t send_cnt;

uint8_t send_idx;
uint8_t send_bitidx;
uint8_t send_nextbit;

#define BIT(x, y) (((x) >> (7-y)) & 1)

void spi_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

	RCC->AHBENR |= RCC_AHBENR_DMA1EN;


	NVIC_SetPriority(DMA1_Channel5_IRQn, 0);
	NVIC_EnableIRQ(DMA1_Channel5_IRQn);

	NVIC_SetPriority(TIM1_UP_IRQn, 0);
	NVIC_EnableIRQ(TIM1_UP_IRQn);

	// initialize timer 1us/4 period
	uint32_t a = SystemFrequency / (4*1000*1000);
	TIM1->PSC = 1 - 1; // prescaler
	TIM1->ARR = a - 1; // period
	TIM1->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;

	// Init DMA Channel (MEM->SPI)
	DMA1_Channel5->CPAR = (uint32_t)&TIM2->CCR4;
	DMA1_Channel5->CCR = (0x3 << 12) | (1 << 10) | (1 << 8) | DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TCIE;
	TIM1->DIER |= TIM_DIER_UDE;
	//TIM2->DIER |= TIM_DIER_UDE;
}

void spi_send(uint8_t *buf, uint32_t size)
{

	if (size >= 256) while(1);

	while (done == 0);
	done = 0;

	TIM2->CCR4 = 0;

	memset(send_buf, 0, sizeof(send_buf));

	unsigned int lastbit = 0;
	unsigned int i,j,k;
	for (i = 0; i < size; i++) {
		for (j = 0; j < 8; j++) {
			int bit =  BIT(buf[i], j);

			unsigned int cnt = 4;
			// on transition from 1 to 0 '1' needs to be shortened by 1/4 of bit
			if (lastbit == 1 && bit == 0) {
				cnt = 3;
			}

			for (k = 0; k < cnt; k++) {
				send_buf[4*(i*8+j)+k] = lastbit?2:0;
			}

			lastbit = bit;
		}
	}

	DMA1_Channel5->CMAR = (uint32_t)&send_buf;
	DMA1_Channel5->CNDTR = size*8*4;
	DMA1_Channel5->CCR |= DMA_CCR_EN;

	TIM1->EGR |= TIM_EGR_UG;
	TIM1->CR1 |= TIM_CR1_CEN;

	while (done == 0);
}

void DMA1_Channel5_IRQHandler(void)
{
	DMA1->IFCR = DMA_IFCR_CGIF5;
	DMA1_Channel5->CCR &= ~DMA_CCR_EN;

	done = 1;

	TIM2->CCR4 = 0;
	TIM1->CR1 &= ~TIM_CR1_CEN;
}
