/*
 * SPI Slave driver for STM32 family processors
 *
 * 2010 Michal Demin
 *
 */

#ifndef SU_COMM_H_
#define SU_COMM_H_

void su_comm_init(void);
void su_comm_send(uint8_t *buf, uint32_t size);

#endif

