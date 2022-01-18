/*
 * SPI Slave driver for STM32 family processors
 *
 * 2010 Michal Demin
 *
 */

#ifndef SPI_H_
#define SPI_H_

void spi_init(void);
void spi_send(uint8_t *buf, uint32_t size);

#endif

