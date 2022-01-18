
#include "platform.h"

#include "su_comm.h"
#include "su_mainpll.h"


#define CNT_N_GO (1 << 18)
#define MAIN_P   32

uint16_t su_mainpll_freq = 1928; 

enum {
	PLL_CNT_R = 0b00,
	PLL_CNT_N = 0b01,
	PLL_FUNC =  0b10,
	PLL_INIT =  0b11,
};

uint8_t su_mainpll_data[] = {
	0b11110001,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000, // spacing between messages
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
};

static void su_mainpll_write(uint8_t ctl, uint32_t data);

void su_mainpll_init(void)
{
	// taken from sniff
	su_mainpll_write(PLL_INIT, 0b000000010001101100);

	// testmode, pwrdn mode, testmodes, timeout cntr, CP-tristate = 0
	// phase detector polarity = 1,
	// FoLD = 100 - Digital lock detect
	// powerdown, counterreset = 0
	su_mainpll_write(PLL_FUNC, 0b000000000000100100);

	// R = 20
	su_mainpll_write(PLL_CNT_R, 20);

	su_mainpll_tune(1928);
}

void su_mainpll_tune(uint16_t freq)
{
	uint16_t a;
	uint16_t b;

	// TODO: account for R
	b = freq / MAIN_P;
	a = freq - (b * MAIN_P);

	a &= 0x1f;
	b &= 0x1fff;

	su_mainpll_write(PLL_CNT_N, CNT_N_GO | (b << 5) | a);

	su_mainpll_freq = freq;
}

static void su_mainpll_write(uint8_t ctl, uint32_t data)
{
	ctl = ctl & 0x03;
	data = data & 0x7ffff;

	data = (data << 3) | (ctl << 1);

	su_mainpll_data[1] = (data >> 16) & 0xff;
	su_mainpll_data[2] = (data >> 8) & 0xff;
	su_mainpll_data[3] = data & 0xff;

	su_comm_send(su_mainpll_data, ARRAY_SIZE(su_mainpll_data));
}

