#ifndef SU_MAINPLL_h_
#define SU_MAINPLL_h_

extern uint16_t su_mainpll_freq;

void su_mainpll_init();
void su_mainpll_tune(uint16_t freq);

#endif
