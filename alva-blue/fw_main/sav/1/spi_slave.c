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

uint16_t send_buf[256];

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

	// initialize timer 1us period
	uint32_t a = SystemFrequency / (1000 * 1000);
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
	while (done == 0);
	done = 0;
/*
	send_buf = buf;
	send_cnt = size;
	send_idx = 0;
	send_bitidx = 1;
	send_nextbit = BIT(send_buf[0], 0);

	TIM1->DIER |= TIM_DIER_UIE;
*/
	TIM2->CCR4 = 0;

	memset(send_buf, 0, sizeof(send_buf));
	for (unsigned int i = 0; i < size; i++) {
		for (unsigned int j = 0; j < 8; j++) {
			send_buf[i*8+j] = BIT(buf[i], j)?2:0;
		}
	}

	DMA1_Channel5->CMAR = (uint32_t)&send_buf;
	DMA1_Channel5->CNDTR = size*8;
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
/*
void TIM1_UP_IRQHandler(void)
{
	TIM1->SR &= ~TIM_SR_UIF;

	if (UNLIKELY(done == 1)) {
		TIM2->CCER &= ~TIM_CCER_CC4E;
		TIM1->DIER &= ~TIM_DIER_UIE;
		return;
	}

	if (send_nextbit) {
		TIM2->CCER |= TIM_CCER_CC4E;
	} else {
		TIM2->CCER &= ~TIM_CCER_CC4E;
	}

	send_nextbit = BIT(send_buf[send_idx], send_bitidx);
	send_bitidx++;
	if (send_bitidx == 8) {
		send_idx++;
		send_bitidx = 0;
		if (send_idx == send_cnt) {
			done = 1;
		}
	}
}*/
