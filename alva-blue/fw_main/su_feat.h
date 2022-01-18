#ifndef SU_FEAT_h_
#define SU_FEAT_h_

enum {
	SU_FEAT_SWITCH  = 0b00000100, // 0x04
	SU_FEAT_TXEN    = 0b00001100, // 0x0c
	SU_FEAT_LATCH_1 = 0b01111010, // 0x7A
	SU_FEAT_LATCH_2 = 0b01111100, // 0x7C
};

extern uint16_t su_feat_latch1;
extern uint16_t su_feat_latch2;

void su_feat_init(void);
void su_feat_set_rxtx(uint8_t tx);
uint8_t su_feat_get_rxtx(void);
void su_feat_set_txen(uint8_t on);
uint8_t su_feat_get_txen(void);
void su_feat_set_txpreamp(uint8_t bias);
uint8_t su_feat_get_txpreamp(void);
void su_feat_set_rxpreamp(uint8_t on);
int su_feat_get_rxpreamp(void);
void su_feat_set_led(uint16_t led);
uint16_t su_feat_get_led(void);
void su_feat_write(uint8_t feat, uint16_t data, uint16_t data2);
void su_feat_write1(uint8_t feat);


#endif
