
#include "platform.h"

#include "su_comm.h"
#include "su_feat.h"

/*
LED 1-9 = orange
led A = red
LED E = eth
led W = wlnk
P = preamp

latch1
	            LEDS
	         8765 4321   P??? WEA9
	01111010 0001 0100   0000 1100 00... # 24 after pll set
	         1111 1110   1100 1111 00... # strobe, then write 1.9ghz pll

latch2
	           B
	         ?123 4???   not used
	01111100 0111 1011   0000 0000 ... # 15

	??? - n/c, unknown

	TX preamp bias
	1234: 0-7 -> (0,-2.68)Volt
	P preamp enable

*/

uint8_t su_feat_bias = 0;
uint8_t su_feat_txen = 0;
uint8_t su_feat_rxtx = 0;

uint16_t su_feat_latch1 = 0xfecf;
uint16_t su_feat_latch2 = 0x7000;


uint8_t su_feat_data[] = {
	0b01111010,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
};


void su_feat_init(void)
{
	su_feat_write(SU_FEAT_LATCH_1, su_feat_latch1, 0);
	su_feat_write(SU_FEAT_LATCH_2, su_feat_latch2, 0);
}

// if switch
void su_feat_set_rxtx(uint8_t tx)
{
	su_feat_rxtx = tx;

	tx = !!tx;
	su_feat_write1(SU_FEAT_SWITCH | tx);
}

uint8_t su_feat_get_rxtx(void)
{
	return su_feat_rxtx;
}

// transmitter enable
void su_feat_set_txen(uint8_t on)
{
	su_feat_txen = on;

	on = !on;
	su_feat_write1(SU_FEAT_TXEN | on);
}

uint8_t su_feat_get_txen(void)
{
	return su_feat_txen;
}

void su_feat_set_txpreamp(uint8_t bias)
{
	su_feat_bias = bias;

	bias &= 0x0f;

	su_feat_latch2 &= 0x7800;
	su_feat_latch2 |= (bias << (3+8));

	su_feat_write(SU_FEAT_LATCH_2, su_feat_latch2, 0);
}

uint8_t su_feat_get_txpreamp(void)
{
	return (su_feat_latch2 & 0x7800) >> (3+8);
}

void su_feat_set_rxpreamp(uint8_t on)
{
	su_feat_latch1 &= 0xff3f;

	if (on) {
		su_feat_latch1 |= 0x00c0;
	}

	su_feat_write(SU_FEAT_LATCH_1, su_feat_latch1, 0);
}

int su_feat_get_rxpreamp(void)
{
	return !!(su_feat_latch1 & 0x00c0);
}

void su_feat_set_led(uint16_t led)
{
	led = (~led) & 0xfff;
	led = (led >> 8) | (led << 8);

	su_feat_latch1 &= 0x00f0;
	su_feat_latch1 |= led;

	su_feat_write(SU_FEAT_LATCH_1, su_feat_latch1, 0);
}

uint16_t su_feat_get_led(void)
{
	return ((su_feat_latch1 >> 8) | (su_feat_latch1 << 8)) & 0xfff;
}

void su_feat_write(uint8_t feat, uint16_t data, uint16_t data2)
{
	su_feat_data[0] = feat;
	su_feat_data[1] = (data >> 8) & 0xff;
	su_feat_data[2] = data & 0xff;
	su_feat_data[3] = (data2 >> 8) & 0xff;
	su_feat_data[4] = data2 & 0xff;

	su_comm_send(su_feat_data, ARRAY_SIZE(su_feat_data));
}

void su_feat_write1(uint8_t feat)
{
	su_feat_data[0] = feat;
	su_feat_data[1] = 0x00;
	su_feat_data[2] = 0x00;
	su_feat_data[3] = 0x00;
	su_feat_data[4] = 0x00;

	su_comm_send(su_feat_data, ARRAY_SIZE(su_feat_data));
}
