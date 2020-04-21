/**
 * @file tp_spi.h
 *
 */

#ifndef TP_SPI_H
#define TP_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <driver/spi_master.h>


/*********************
 *      DEFINES
 *********************/

// SPI Bus when using tp_spi_init()
#define TOUCH_SPI_HOST VSPI_HOST

// Touchscreen GPIO pins
#define TP_SPI_MOSI 18
#define TP_SPI_MISO 19
#define TP_SPI_CLK  5
#define TP_SPI_CS   32


/**********************
 *      TYPEDEFS
 **********************/


/**********************
 * GLOBAL PROTOTYPES
 **********************/
void tp_spi_init(void);
void tp_spi_add_device(spi_host_device_t host);
void tp_spi_add_device_config(spi_host_device_t host, spi_device_interface_config_t *config);
void tp_spi_xchg(uint8_t* data_send, uint8_t* data_recv, uint8_t byte_count);
void tp_spi_write_reg(uint8_t* data, uint8_t byte_count);
void tp_spi_read_reg(uint8_t reg, uint8_t* data, uint8_t byte_count);


/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*TP_SPI_H*/
