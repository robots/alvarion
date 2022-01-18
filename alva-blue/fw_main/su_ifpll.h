#ifndef SU_IFPLL_h_
#define SU_IFPLL_h_

enum {
	IFPLL_AUX_R =  0b00000000,
	IFPLL_AUX_N =  0b01000000,
	IFPLL_MAIN_R = 0b10000000,
	IFPLL_MAIN_N = 0b11000000,
};

enum {
	IFPLL_TX = 0b00001000,
	IFPLL_RX = 0b00010000,
};

extern uint16_t su_ifpll_rx_freq;
extern uint16_t su_ifpll_tx_freq;
extern uint16_t su_ifpll_aux_freq;

void su_ifpll_init(void);
void su_ifpll_maintune(uint8_t pll, uint16_t freq);
void su_ifpll_auxtune(uint16_t freq);

#endif
