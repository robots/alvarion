#include "platform.h"

#include "su_comm.h"
#include "su_ifpll.h"

#define MAIN_P  16
#define AUX_P   8


uint16_t su_ifpll_rx_freq = 1032;
uint16_t su_ifpll_tx_freq = 1135;
uint16_t su_ifpll_aux_freq = 440;

uint8_t su_ifpll_data[] = {
	0b00011110,
	0b00000000, // determines which pll gets written
	0b00000000, // data to pll
	0b00000000, // data to pll
	0b11000000, // 2 ctrl bits, 6 zero padding
	0b00000000, // spacing between messages
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
};

static void su_ifpll_write(uint8_t pll, uint8_t ctl, uint16_t data);

void su_ifpll_init(void)
{
	uint16_t data;

	data = (0b0010 << 12) | 1000; // FoLD = 0010, R = 20
	su_ifpll_write(IFPLL_RX, IFPLL_AUX_R, data);

	data = (2750 << 4) | 0; // B = 2750, A = 0
	su_ifpll_write(IFPLL_RX, IFPLL_AUX_N, data);

	data = (0b1011 << 12) | 20; // CP_WORD = 1011, R = 20
	su_ifpll_write(IFPLL_RX, IFPLL_MAIN_R, data);
	
	data = (70 << 4) | 12; // B = 70, A = 12
	su_ifpll_write(IFPLL_RX, IFPLL_MAIN_N, data);

	data = (0b1011 << 12) | 20; // CP_WORD = 1011, R = 20
	su_ifpll_write(IFPLL_TX, IFPLL_MAIN_R, data);

	data = (64 << 4) | 8; // B = 64, A = 8
	su_ifpll_write(IFPLL_TX, IFPLL_MAIN_N, data);

	data = (0b0010 << 12) | 0; // FoLD = 0010, R = 0
	su_ifpll_write(IFPLL_TX, IFPLL_AUX_R, data);
}

void su_ifpll_maintune(uint8_t pll, uint16_t freq)
{
	uint16_t data;
	uint16_t a;
	uint16_t b;

	b = freq / MAIN_P;
	a = freq - (b * MAIN_P);

	a = a & 0x000f;
	b = b & 0x0fff;

	data = (b << 4) | a;
	su_ifpll_write(pll, IFPLL_MAIN_N, data);

	if (pll == IFPLL_RX) {
		su_ifpll_rx_freq = freq;
	} else if (pll == IFPLL_TX) {
		su_ifpll_tx_freq = freq;
	}
}

void su_ifpll_auxtune(uint16_t freq)
{
	uint16_t data;
	uint16_t a;
	uint16_t b;

	uint32_t tmp;

	tmp = freq * 1000;
	tmp /= 20;

	b = tmp / AUX_P;
	a = tmp - (b * AUX_P);

	a = a & 0x000f;
	b = b & 0x0fff;

	data = (b << 4) | a;
	su_ifpll_write(IFPLL_RX, IFPLL_AUX_N, data);
	su_ifpll_aux_freq = freq;
}

static void su_ifpll_write(uint8_t pll, uint8_t ctl, uint16_t data)
{
	su_ifpll_data[1] = pll;
	su_ifpll_data[2] = data >> 8;
	su_ifpll_data[3] = data & 0xff;
	su_ifpll_data[4] = ctl;

	su_comm_send(su_ifpll_data, ARRAY_SIZE(su_ifpll_data));
}

