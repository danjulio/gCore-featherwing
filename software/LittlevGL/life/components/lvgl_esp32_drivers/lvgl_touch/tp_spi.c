/**
 * @file tp_spi.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "tp_spi.h"
#include "touch_driver.h"
#include "esp_system.h"
#include "driver/spi_master.h"


/*********************
 *      DEFINES
 *********************/


/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/
static spi_device_handle_t spi;


/**********************
 *  STATIC VARIABLES
 **********************/


/**********************
 *      MACROS
 **********************/


/**********************
 *   GLOBAL FUNCTIONS
 **********************/
 
void tp_spi_add_device_config(spi_host_device_t host, spi_device_interface_config_t *devcfg)
{
	esp_err_t ret=spi_bus_add_device(host, devcfg, &spi);
	assert(ret==ESP_OK);
}


void tp_spi_add_device(spi_host_device_t host)
{
	spi_device_interface_config_t devcfg={
		.clock_speed_hz=1*1000*1000,           //Clock out at 1 MHz
		.mode=1,                               //SPI mode 1
		.spics_io_num=TP_SPI_CS,               //CS pin
		.queue_size=1,
		.pre_cb=NULL,
		.post_cb=NULL,
	};
	
	//Attach the Touch controller to the SPI bus
	tp_spi_add_device_config(host, &devcfg);
}


void tp_spi_init(void)
{
	esp_err_t ret;

	spi_bus_config_t buscfg={
		.miso_io_num=TP_SPI_MISO,
		.mosi_io_num=TP_SPI_MOSI,
		.sclk_io_num=TP_SPI_CLK,
		.quadwp_io_num=-1,
		.quadhd_io_num=-1
	};

	//Initialize the SPI bus
	ret=spi_bus_initialize(TOUCH_SPI_HOST, &buscfg, 2);
	assert(ret==ESP_OK);

	//Attach the Touch controller to the SPI bus
	tp_spi_add_device(TOUCH_SPI_HOST);
}


void tp_spi_xchg(uint8_t* data_send, uint8_t* data_recv, uint8_t byte_count)
{
	spi_transaction_t t = {
		.length = byte_count * 8, // SPI transaction length is in bits
		.tx_buffer = data_send,
		.rx_buffer = data_recv};
	
	esp_err_t ret = spi_device_transmit(spi, &t);
	assert(ret == ESP_OK);
}


void tp_spi_write_reg(uint8_t* data, uint8_t byte_count)
{
	spi_transaction_t t;
	
	memset(&t, 0, sizeof(t));
	
	t.length = byte_count * 8;
	t.tx_buffer = data;
	t.flags = SPI_DEVICE_HALFDUPLEX;
	esp_err_t ret = spi_device_transmit(spi, &t);
	assert(ret == ESP_OK);
}


void tp_spi_read_reg(uint8_t reg, uint8_t* data, uint8_t byte_count)
{
	spi_transaction_t t;
	spi_transaction_ext_t et;
	
	memset(&t, 0, sizeof(t));
	
	// Read - send first byte as command
	t.length = byte_count * 8;
	t.cmd = reg;
	t.rx_buffer = data;
	t.flags = SPI_TRANS_VARIABLE_CMD | SPI_DEVICE_HALFDUPLEX;
	et.base = t;
	et.command_bits = 8;
	et.address_bits = 0;
	esp_err_t ret = spi_device_transmit(spi, (spi_transaction_t*)&et);
	assert(ret == ESP_OK);
}


/**********************
 *   STATIC FUNCTIONS
 **********************/
