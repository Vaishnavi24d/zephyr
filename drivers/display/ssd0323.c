/*
 * Copyright (c) 2018 Zilogic Systems.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_LEVEL CONFIG_DISPLAY_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(ssd0323);

#include <device.h>
#include <display.h>
#include <gpio.h>
#include <spi.h>
#include "ssd0323_regs.h"

struct ssd0323_data {
	struct device *dc;
	struct device *spi_dev;
	struct spi_config spi_config;
#if defined(DT_SSD0323_CS_GPIO_PORT_NAME)
	struct spi_cs_control cs_ctrl;
#endif
	u8_t contrast;
};

static inline int ssd0323_write_cmd(struct ssd0323_data *driver,
				    u8_t cmd, u8_t *data, size_t len)
{
	struct spi_buf buf = {.buf = &cmd, .len = sizeof(cmd)};
	struct spi_buf_set buf_set = {.buffers = &buf, .count = 1};

	gpio_pin_write(driver->dc, DT_SSD0323_DC_PIN, 0);
	if (spi_write(driver->spi_dev, &driver->spi_config, &buf_set)) {
		LOG_ERR("Command write using SPI failed");
		return -EIO;
	}

	if (data != NULL) {
		buf.buf = data;
		buf.len = len;
		if (spi_write(driver->spi_dev, &driver->spi_config, &buf_set)) {
			LOG_ERR("Command write using SPI failed");
			return -EIO;
		}
	}

	return 0;
}

static int ssd0323_set_orientation(const struct device *dev,
				   const enum display_orientation orientation)
{
	LOG_ERR("not supported");
	return -ENOTSUP;
}

static int ssd0323_resume(const struct device *dev)
{
	struct ssd0323_data *driver = dev->driver_data;

	return ssd0323_write_cmd(driver, SSD0323_CMD_DISP_ON,
				 0, 0);
}

static int ssd0323_suspend(const struct device *dev)
{
	struct ssd0323_data *driver = dev->driver_data;

	return ssd0323_write_cmd(driver, SSD0323_CMD_DISP_OFF,
				 0, 0);
}

static int ssd0323_write(const struct device *dev, const u16_t x,
			 const u16_t y,
			 const struct display_buffer_descriptor *desc,
			 const void *buf)
{
	struct ssd0323_data *driver = dev->driver_data;
	struct spi_buf sbuf = {.buf = buf, .len = desc->buf_size};
	struct spi_buf_set buf_set = {.buffers = &sbuf, .count = 1};
	u8_t x_range[2];
	u8_t y_range[2];
	int ret;

	assert(buf != NULL);
	assert(desc->buf_size != 0);

	y_range[0] = y;
	y_range[1] = SSD0323_ROW_END;

	ssd0323_write_cmd(driver, SSD0323_CMD_SET_ROW_ADDR,
			  y_range, sizeof(y_range));

	if (desc->pitch != 0 && desc->pitch < DT_SSD0323_PANEL_WIDTH) {
		x_range[1] = (x / 2) + ((desc->pitch) / 2) - 1;
	} else {
		x_range[1] = SSD0323_COLUMN_END;
	}

	x_range[0] = (x / 2);

	ssd0323_write_cmd(driver, SSD0323_CMD_SET_COL_ADDR,
			  x_range, sizeof(x_range));

	ret = gpio_pin_write(driver->dc, DT_SSD0323_DC_PIN, 1);
	if (ret) {
		LOG_ERR("GPIO write to DC pin failed");
		return -EIO;
	}

	ret = spi_write(driver->spi_dev, &driver->spi_config, &buf_set);
	if (ret) {
		LOG_ERR("Data writing using SPI failed");
		return -EIO;
	}

	ret = gpio_pin_write(driver->dc, DT_SSD0323_DC_PIN, 0);
	if (ret) {
		LOG_ERR("GPIO write to DC pin failed");
		return -EIO;
	}

	return 0;
}

static int ssd0323_read(const struct device *dev, const u16_t x,
			const u16_t y,
			const struct display_buffer_descriptor *desc,
			void *buf)
{
	LOG_ERR("not supported");
	return -ENOTSUP;
}

static void *ssd0323_get_framebuffer(const struct device *dev)
{
	LOG_ERR("not supported");
	return NULL;
}

static int ssd0323_set_brightness(const struct device *dev,
				  const u8_t brightness)
{
	LOG_WRN("not supported");
	return -ENOTSUP;
}

static int ssd0323_set_contrast(const struct device *dev, u8_t contrast)
{
	struct ssd0323_data *driver = dev->driver_data;

	return ssd0323_write_cmd(driver, SSD0323_CMD_SET_CONTRAST,
				 &contrast, sizeof(contrast));
}

static void ssd0323_get_capabilities(const struct device *dev,
				     struct display_capabilities *caps)
{
	memset(caps, 0, sizeof(struct display_capabilities));
	caps->x_resolution = DT_SSD0323_PANEL_WIDTH;
	caps->y_resolution = DT_SSD0323_PANEL_HEIGHT;
	caps->supported_pixel_formats = PIXEL_FORMAT_GRAY_4BPP;
	caps->current_pixel_format = PIXEL_FORMAT_GRAY_4BPP;
	caps->screen_info = SCREEN_INFO_MONO_MSB_FIRST;
	caps->current_orientation = DISPLAY_ORIENTATION_NORMAL;
}

static int ssd0323_set_pixel_format(const struct device *dev,
				    const enum display_pixel_format pf)
{
	if (pf != PIXEL_FORMAT_GRAY_4BPP) {
		LOG_ERR("pixel format not supported");
		return -ENOTSUP;
	}

	return 0;
}

static int ssd0323_init(struct device *dev)
{
	struct ssd0323_data *driver = dev->driver_data;
	int ret;

	driver->spi_dev = device_get_binding(DT_SSD0323_SPI_DEV_NAME);
	if (driver->spi_dev == NULL) {
		LOG_ERR("Could not get SPI device for SSD0323");
		return -EIO;
	}

	driver->spi_config.frequency = DT_SSD0323_SPI_FREQ;
	driver->spi_config.operation = SPI_OP_MODE_MASTER | SPI_MODE_CPOL |
		SPI_MODE_CPHA | SPI_WORD_SET(8) | SPI_LINES_SINGLE;
	driver->spi_config.cs = NULL;

	driver->dc = device_get_binding(DT_SSD0323_DC_GPIO_PORT_NAME);
	if (driver->dc == NULL) {
		LOG_ERR("Could not get GPIO port for SSD0323 DC signal");
		return -EIO;
	}

	ret = gpio_pin_configure(driver->dc, DT_SSD0323_DC_PIN,
				 GPIO_DIR_OUT);
	if (ret < 0) {
		LOG_ERR("Error configuring GPIO");
		return -EIO;
	}

#if defined(DT_SSD0323_CS_GPIO_PORT_NAME)
	driver->cs_ctrl.gpio_dev = device_get_binding(
		DT_SSD0323_CS_GPIO_PORT_NAME);
	if (!driver->cs_ctrl.gpio_dev) {
		LOG_ERR("Unable to get SPI GPIO CS device");
		return -EIO;
	}

	driver->cs_ctrl.gpio_pin = DT_SSD0323_CS_PIN;
	driver->cs_ctrl.delay = 0;
	driver->spi_config.cs = &driver->cs_ctrl;
#endif
	return 0;
}

static struct ssd0323_data ssd0323_driver_data;

static struct display_driver_api ssd0323_driver_api = {
	.blanking_on = ssd0323_resume,
	.blanking_off = ssd0323_suspend,
	.write = ssd0323_write,
	.read = ssd0323_read,
	.get_framebuffer = ssd0323_get_framebuffer,
	.set_brightness = ssd0323_set_brightness,
	.set_contrast = ssd0323_set_contrast,
	.get_capabilities = ssd0323_get_capabilities,
	.set_pixel_format = ssd0323_set_pixel_format,
	.set_orientation = ssd0323_set_orientation,
};

DEVICE_AND_API_INIT(ssd0323, DT_SSD0323_DEV_NAME, ssd0323_init,
		    &ssd0323_driver_data, NULL,
		    POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY,
		    &ssd0323_driver_api);
