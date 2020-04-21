/**
 * @file touch_driver.c
 */
#include "touch_driver.h"
#include "tp_spi.h"


void touch_driver_init(bool init_spi)
{
	if (init_spi) {
    	tp_spi_init();
    }
	stmpe610_init();
}


bool touch_driver_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    bool res = false;

	res = stmpe610_read(drv, data);

    return res;
}

