/**
 * @file disp_spi.h
 *
 */

#ifndef DISP_SPI_H
#define DISP_SPI_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <stdbool.h>
#include <driver/spi_master.h>
#include "lv_conf.h"



/*********************
 *      DEFINES
 *********************/
 
// SPI Bus when using disp_spi_init()
 #define TFT_SPI_HOST VSPI_HOST

// Buffer size - sets maximum update region (and can use a lot of memory!)
#define DISP_BUF_SIZE (LV_HOR_RES_MAX * 40)
 
// Display-specific GPIO
#define DISP_SPI_MOSI 18
#define DISP_SPI_CLK  5
#define DISP_SPI_CS   15


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void disp_spi_init(void);
void disp_spi_add_device(spi_host_device_t host);
void disp_spi_add_device_config(spi_host_device_t host, spi_device_interface_config_t *devcfg);
void disp_spi_send_data(uint8_t * data, uint16_t length);
void disp_spi_send_colors(uint8_t * data, uint16_t length);
bool disp_spi_is_busy(void);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DISP_SPI_H*/
