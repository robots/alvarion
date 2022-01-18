#include "platform.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "systime.h"


#include "gpio.h"
#include "su_comm.h"
#include "clkgen.h"
#include "su_ifpll.h"
#include "su_mainpll.h"
#include "su_feat.h"
#include "su.h"

#include "console.h"

#include "LiquidCrystal_I2C.h"

static struct console_command_t su_cmd;

static char line1[17];
static char line2[17];
volatile int lcd_update;

uint32_t main_pll_tx = 1933; 
uint32_t main_pll_rx = 1830; 

void su_rxtx(int tx);

const struct gpio_init_table_t su_gpio[] = {
	{ // ptt out
		.gpio = GPIOB,
		.pin = GPIO_Pin_13,
		.mode = GPIO_MODE_OUT_OD,
		.speed = GPIO_SPEED_HIGH,
		.state = GPIO_SET,
	},
	{ // ptt in
		.gpio = GPIOB,
		.pin = GPIO_Pin_14,
		.mode = GPIO_MODE_IPU,
		.speed = GPIO_SPEED_HIGH,
	},
};

void su_init(void)
{
	gpio_init(su_gpio, ARRAY_SIZE(su_gpio));

	su_comm_init();

	su_feat_init();

	// set relay to known position
	su_feat_set_rxtx(1);
	systime_delay(300);
	su_feat_set_rxtx(0);
	systime_delay(300);

	su_feat_set_txen(0);

	su_mainpll_init();
	su_mainpll_tune(1928);

	su_ifpll_init();
	su_ifpll_maintune(IFPLL_TX, 1032);
  su_ifpll_maintune(IFPLL_RX, 1135);
  su_ifpll_auxtune(440);

/*	int x=0;
	while (1) {
		systime_delay(1);
		su_feat_led(x);
		x++;
		x&=0xff;
	}
*/

	console_add_command(&su_cmd);
	lcd_update = 1;
}

void su_periodic(void)
{
	static uint32_t last_time = 0;
	static uint32_t last_ptt = 2;

	if (systime_get() - last_time > 500) {
		uint32_t ptt = gpio_get(&su_gpio[1]);
		if (ptt != last_ptt) {

			if (ptt == 0) {
				su_feat_set_led(1);
				// set transverter to TX
				su_rxtx(1);
				su_feat_set_led(2);
				// "press" ptt on the station
				gpio_set(&su_gpio[0], GPIO_RESET);
			} else {
				// "release" ptt first
				gpio_set(&su_gpio[0], GPIO_SET);
				su_feat_set_led(3);
				// set transverter to RX
				su_rxtx(0);
				su_feat_set_led(4);
			}


			last_ptt = ptt;
		}

		last_time = systime_get();
	}


	if (lcd_update) {
		lcd_update = 0;

		sprintf(line1, "RX:%04d TX:%04d", su_ifpll_rx_freq + main_pll_rx + 440, su_ifpll_tx_freq + main_pll_tx + 440);
		sprintf(line2, "M:%04d B:%1d %c %c", 0, su_feat_get_txpreamp(), su_feat_get_txen()?'T':' ', su_feat_get_rxtx()?'T':'R');

		// test ptt button, switch rx/tx
		LCDI2C_setCursor(0,0);
		LCDI2C_write_String(line1);
		LCDI2C_setCursor(0,1);
		LCDI2C_write_String(line2);
	}

}

void su_rxtx(int tx)
{
	if (tx) {
		su_feat_set_rxpreamp(0);
		systime_delay(10);

		su_feat_set_rxtx(1);
		// wait for relay
		systime_delay(300);
		su_mainpll_tune(main_pll_tx);
		systime_delay(10);

		su_feat_set_txpreamp(1);
		su_feat_set_txen(1);
	} else {
		su_feat_set_txen(0);
		systime_delay(10);

		su_feat_set_rxtx(0);

		// wait for relay
		systime_delay(300);
		
		su_mainpll_tune(main_pll_rx);
		systime_delay(10);

		su_feat_set_rxpreamp(1);
	}

	lcd_update = 1;
}

static uint8_t su_cmd_handler(struct console_session_t *cs, char **args)
{
	if (args[0] == NULL) {
		console_session_printf(cs, "Valid: main x, txif x, rxif x, aux x, led x, feat1 x, feat2 x, switch x, tx x, txamp x, rxamp x\n");
		return 1;
	}

	if (strcmp(args[0], "main") == 0) {
		uint32_t x;
		x = strtoll(args[1], NULL, 0);
		console_session_printf(cs, "Tune main PLL to %d\n", x);
		su_mainpll_tune(x);
	} else if (strcmp(args[0], "txif") == 0) {
		uint32_t x;
		x = strtoll(args[1], NULL, 0);
		console_session_printf(cs, "Tune TX IF PLL to %d\n", x);
		su_ifpll_maintune(IFPLL_TX, x);
	} else if (strcmp(args[0], "rxif") == 0) {
		uint32_t x;
		x = strtoll(args[1], NULL, 0);
		console_session_printf(cs, "Tune RX IF PLL to %d\n", x);
		su_ifpll_maintune(IFPLL_TX, x);
	} else if (strcmp(args[0], "aux") == 0) {
		uint32_t x;
		x = strtoll(args[1], NULL, 0);
		console_session_printf(cs, "Tune RX AUX PLL to %d\n", x);
		su_ifpll_auxtune(x);
	} else if (strcmp(args[0], "led") == 0) {
		uint32_t x;
		x = strtoll(args[1], NULL, 0);
		console_session_printf(cs, "Setting led to %x\n", x);
		su_feat_set_led(x);
	} else if (strcmp(args[0], "feat1") == 0) {
		uint32_t x;
		x = strtoll(args[1], NULL, 0);
		console_session_printf(cs, "Setting latch1 to %x\n", x);
		su_feat_write(SU_FEAT_LATCH_1, x & 0xffff, x >> 16);
	} else if (strcmp(args[0], "feat2") == 0) {
		uint32_t x;
		x = strtoll(args[1], NULL, 0);
		console_session_printf(cs, "Setting latch2 to %x\n", x);
		su_feat_write(SU_FEAT_LATCH_2, x & 0xffff, x >> 16);
	} else if (strcmp(args[0], "switch") == 0) {
		uint32_t x;
		x = strtoll(args[1], NULL, 0);
		console_session_printf(cs, "Setting switch to %x\n", x);
		su_feat_set_rxtx(x);
	} else if (strcmp(args[0], "tx") == 0) {
		uint32_t x;
		x = strtoll(args[1], NULL, 0);
		console_session_printf(cs, "Setting TX to %x\n", x);
		su_feat_set_txen(x);
	} else if (strcmp(args[0], "txamp") == 0) {
		uint32_t x;
		x = strtoll(args[1], NULL, 0);
		console_session_printf(cs, "Setting TXamp to %d\n", x);
		su_feat_set_txpreamp(x);
	} else if (strcmp(args[0], "rxamp") == 0) {
		uint32_t x;
		x = strtoll(args[1], NULL, 0);
		console_session_printf(cs, "Setting rxamp to %d\n", x);
		su_feat_set_rxpreamp(x);
	}

	lcd_update = 1;

	return 0;
}

static struct console_command_t su_cmd = {
	"su",
	su_cmd_handler,
	"su",
	"su",
	NULL
};
