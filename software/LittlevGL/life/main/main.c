/* gCore runs "Life"
 * 
 * A tribute to John Conway (1938-2020) who gave us "Life" and who passed away
 * while I was writing the documentation for gCore.
 *
 * This example code is in the Public Domain (or CC0 licensed, at your option.)
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "esp_system.h"

// Application specific
#include "gcore_power.h"
#include "gui.h"
#include "life.h"

// Littlevgl specific
#include "lvgl/lvgl.h"
#include "disp_spi.h"
#include "disp_driver.h"
#include "tp_spi.h"
#include "touch_driver.h"


//
// Defines
//

// Shared SPI Bus for the TFT and Touchscreen
#define GCORE_SPI_HOST VSPI_HOST



//
// Forward declarations
//
static void IRAM_ATTR lv_tick_task(void);
static void driver_init();
static void configure_shared_spi_bus(void);


//
// Application entry point
//
void app_main() {

	// Setup gCore utility monitoring (switch, charge, battery)
	gcore_begin();
	
	// Setup LittlevGL
	lv_init();
	driver_init();
	esp_register_freertos_tick_hook(lv_tick_task);

	// Create the GUI 
	gui_init();

	// Evaluate LittlevGL (GUI containing life evaluation)
	while (1) {
		vTaskDelay(5);
		lv_task_handler();
	}
}



//
// Subroutines
//
 
static void IRAM_ATTR lv_tick_task(void) {
	lv_tick_inc(portTICK_RATE_MS);
}


static void driver_init()
{
	//
	// Interface and driver initialization
	//   Configure one SPI bus for the two devices
	//   Configure the drivers (they don't need to initialize their SPI bus)
	//
	configure_shared_spi_bus();
	disp_driver_init(false);
	touch_driver_init(false);

	//
	// Register drivers with LittlevGL
	//
	static lv_color_t buf1[DISP_BUF_SIZE];
	static lv_color_t buf2[DISP_BUF_SIZE];
	static lv_disp_buf_t disp_buf;
	lv_disp_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);

	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.flush_cb = disp_driver_flush;
	disp_drv.buffer = &disp_buf;
	lv_disp_drv_register(&disp_drv);

	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.read_cb = touch_driver_read;
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	lv_indev_drv_register(&indev_drv);
}


static void configure_shared_spi_bus(void)
{
	// Shared SPI bus configuration
	spi_bus_config_t buscfg = {
		.miso_io_num = TP_SPI_MISO,
		.mosi_io_num = DISP_SPI_MOSI,
		.sclk_io_num = DISP_SPI_CLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = DISP_BUF_SIZE * 2
	};

	esp_err_t ret = spi_bus_initialize(GCORE_SPI_HOST, &buscfg, 1);
	assert(ret == ESP_OK);

	// SPI Devices
	disp_spi_add_device(GCORE_SPI_HOST);
	tp_spi_add_device(GCORE_SPI_HOST);
}
