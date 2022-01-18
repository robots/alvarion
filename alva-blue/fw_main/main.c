#include "platform.h"

#include "console.h"
#include "systime.h"
#include "gpio.h"
#include "led.h"

#include "LiquidCrystal_I2C.h"

#include "console_cdc.h"
#include "su.h"

const struct gpio_init_table_t main_gpio[] = {
	{
		.gpio = GPIOA,
		.pin = 0xffff,
		.mode = GPIO_MODE_AIN,
	},
	{
		.gpio = GPIOB,
		.pin = 0xffff,
		.mode = GPIO_MODE_AIN,
	},
	{
		.gpio = GPIOC,
		.pin = 0xffff,
		.mode = GPIO_MODE_AIN,
	},
};

int main(void)
{
	SystemInit();

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;
	AFIO->MAPR |= (0x02 << 24); // only swj

	gpio_init(main_gpio, ARRAY_SIZE(main_gpio));

	systime_init();
	led_init();

	console_init();
	console_cdc_init();

	LCDI2C_init(0x27, 16, 2);
	LCDI2C_backlight();
	LCDI2C_clear();
	LCDI2C_write_String("SU v0.1");
	systime_delay(600);

	su_init();
	led_set(0, LED_3BLINK);	
	
	while (1) {
		systime_periodic();
		led_periodic();
		su_periodic();
	}
} 
