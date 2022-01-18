
#include "platform.h"

#include "gpio.h"

#include "clkgen.h"

const struct gpio_init_table_t clkgen_gpio[] = {
	{ // TIM2 CH4
		.gpio = GPIOA,
		.pin = GPIO_Pin_3,
		.mode = GPIO_MODE_AF_PP,
		.speed = GPIO_SPEED_HIGH,
	},
};

void clkgen_init(uint32_t freq)
{
	uint32_t f;

	gpio_init(clkgen_gpio, ARRAY_SIZE(clkgen_gpio));

	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	f = SystemFrequency / freq;

	// timebase init
	TIM2->PSC = 1 - 1; // prescaler
	TIM2->ARR = f - 1; // period
	TIM2->EGR = TIM_EGR_UG;
	TIM2->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;

	// oc4 init
	TIM2->CCMR2 |= ((0x60 ) << 8)| TIM_CCMR2_OC4FE;// | TIM_CCMR2_OC4PE;// // oc4 to pwm1 mode, 
	TIM2->CCER |= TIM_CCER_CC4E; // polarity high, output enable
	TIM2->CCR4 = 0;//(f / 2);// - 1;

	// pwm enable
	//TIM2->BDTR |= TIM_BDTR_MOE;

	/*
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	// timebase init
	TIM3->PSC = 1 - 1; // prescaler
	TIM3->ARR = f-1;//(SystemFrequency / 1000000)-1;
	TIM3->EGR = TIM_EGR_UG;
	TIM3->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;
	TIM3->CCMR1 |= (0x60 );
	TIM3->CCER |= TIM_CCER_CC1E;
	TIM3->CCR1 = f/2;
	*/
}

void clkgen_deinit(void)
{
	TIM2->CR1 &= ~TIM_CR1_CEN;
}
