/*
 * otm1284a LCD drm_panel driver.
 *
 * Copyright (c) 2019 tmp64 <tmp32x2@gmail.com>
 *
 * Derived from drivers/gpu/drm/panel/panel-sharp-ls043t1le01.c
 * Copyright (C) 2015 Red Hat
 * Copyright (C) 2015 Sony Mobile Communications Inc.
 * Author: Werner Johansson <werner.johansson@sonymobile.com>
 *
 * Based on AUO panel driver by Rob Clark <robdclark@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <linux/backlight.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>

#include <drm/drmP.h>
#include <drm/drm_crtc.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_panel.h>

#include <video/mipi_display.h>

struct tm_otm1284a_panel {
	struct drm_panel base;
	struct mipi_dsi_device *dsi;

	struct backlight_device *backlight;
	struct regulator *supply;
	struct gpio_desc *reset_gpio;

	bool prepared;
	bool enabled;

	const struct drm_display_mode *mode;
};

static inline struct tm_otm1284a_panel *to_tm_otm1284a_panel(struct drm_panel *panel)
{
	return container_of(panel, struct tm_otm1284a_panel, base);
}

static int tm_otm1284a_panel_init(struct tm_otm1284a_panel *sharp_nt)
{
	struct mipi_dsi_device *dsi = sharp_nt->dsi;
	int ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0)
		return ret;

	msleep(120);

	/* Set both MCU and RGB I/F to 24bpp */
	ret = mipi_dsi_dcs_set_pixel_format(dsi, MIPI_DCS_PIXEL_FMT_24BIT |
					(MIPI_DCS_PIXEL_FMT_24BIT << 4));
	if (ret < 0)
		return ret;

	return 0;
}

static int tm_otm1284a_panel_on(struct tm_otm1284a_panel *sharp_nt)
{
	struct mipi_dsi_device *dsi = sharp_nt->dsi;
	//int ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	/*ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0)
		return ret;*/
	
	mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0xFF, (u8[]){ 0x12, 0x84, 0x01 }, 3);
	mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0x80 }, 1);
	mipi_dsi_dcs_write(dsi, 0xFF, (u8[]){ 0x12, 0x84 }, 2);
	mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0x80 }, 1);
	mipi_dsi_dcs_write(dsi, 0xB0, (u8[]){ 0x01 }, 1);
	mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0x81 }, 1);
	mipi_dsi_dcs_write(dsi, 0xB0, (u8[]){ 0x11 }, 1);
	mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0x82 }, 1);
	mipi_dsi_dcs_write(dsi, 0xB0, (u8[]){ 0x01 }, 1);
	mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0x83 }, 1);
	mipi_dsi_dcs_write(dsi, 0xB0, (u8[]){ 0x41 }, 1);
	mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0x84 }, 1);
	mipi_dsi_dcs_write(dsi, 0xB0, (u8[]){ 0x06 }, 1);
	mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0x91 }, 1);
	mipi_dsi_dcs_write(dsi, 0xB0, (u8[]){ 0x9A }, 1);
	mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0xB3 }, 1);
	mipi_dsi_dcs_write(dsi, 0xC5, (u8[]){ 0x84 }, 1);
	mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0xB1 }, 1);
	mipi_dsi_dcs_write(dsi, 0xC6, (u8[]){ 0x02 }, 1);
	mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0xB4 }, 1);
	mipi_dsi_dcs_write(dsi, 0xC6, (u8[]){ 0x10 }, 1);
	mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0x81 }, 1);
	mipi_dsi_dcs_write(dsi, 0xC1, (u8[]){ 0x66 }, 1);
	mipi_dsi_dcs_write(dsi, 0x00, (u8[]){ 0x83 }, 1);
	mipi_dsi_dcs_write(dsi, 0xC1, (u8[]){ 0xB0 }, 1);
	mipi_dsi_dcs_write(dsi, 0x5E, (u8[]){ 0x0C }, 1);
	mipi_dsi_dcs_write(dsi, 0x51, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x53, (u8[]){ 0x24 }, 1);
	mipi_dsi_dcs_write(dsi, 0x55, (u8[]){ 0x00 }, 1);
	mipi_dsi_dcs_write(dsi, 0x11, (u8[]){ 0x00 }, 1);
	msleep(200);
	mipi_dsi_dcs_write(dsi, 0x29, (u8[]){ 0x00 }, 1);

	return 0;
}

static int tm_otm1284a_panel_off(struct tm_otm1284a_panel *sharp_nt)
{
	struct mipi_dsi_device *dsi = sharp_nt->dsi;
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_off(dsi);
	if (ret < 0)
		return ret;

	ret = mipi_dsi_dcs_enter_sleep_mode(dsi);
	if (ret < 0)
		return ret;

	return 0;
}


static int tm_otm1284a_panel_disable(struct drm_panel *panel)
{
	struct tm_otm1284a_panel *sharp_nt = to_tm_otm1284a_panel(panel);

	if (!sharp_nt->enabled)
		return 0;

	if (sharp_nt->backlight) {
		sharp_nt->backlight->props.power = FB_BLANK_POWERDOWN;
		backlight_update_status(sharp_nt->backlight);
	}

	sharp_nt->enabled = false;

	return 0;
}

static int tm_otm1284a_panel_unprepare(struct drm_panel *panel)
{
	struct tm_otm1284a_panel *sharp_nt = to_tm_otm1284a_panel(panel);
	int ret;

	if (!sharp_nt->prepared)
		return 0;

	ret = tm_otm1284a_panel_off(sharp_nt);
	if (ret < 0) {
		dev_err(panel->dev, "failed to set panel off: %d\n", ret);
		return ret;
	}

	regulator_disable(sharp_nt->supply);
	if (sharp_nt->reset_gpio)
		gpiod_set_value(sharp_nt->reset_gpio, 0);

	sharp_nt->prepared = false;

	return 0;
}

static int tm_otm1284a_panel_prepare(struct drm_panel *panel)
{
	struct tm_otm1284a_panel *sharp_nt = to_tm_otm1284a_panel(panel);
	int ret;

	if (sharp_nt->prepared)
		return 0;

	ret = regulator_enable(sharp_nt->supply);
	if (ret < 0)
		return ret;

	msleep(20);

	if (sharp_nt->reset_gpio) {
		gpiod_set_value(sharp_nt->reset_gpio, 1);
		msleep(1);
		gpiod_set_value(sharp_nt->reset_gpio, 0);
		msleep(1);
		gpiod_set_value(sharp_nt->reset_gpio, 1);
		msleep(10);
	}

	ret = tm_otm1284a_panel_init(sharp_nt);
	if (ret < 0) {
		dev_err(panel->dev, "failed to init panel: %d\n", ret);
		goto poweroff;
	}

	ret = tm_otm1284a_panel_on(sharp_nt);
	if (ret < 0) {
		dev_err(panel->dev, "failed to set panel on: %d\n", ret);
		goto poweroff;
	}

	sharp_nt->prepared = true;

	return 0;

poweroff:
	regulator_disable(sharp_nt->supply);
	if (sharp_nt->reset_gpio)
		gpiod_set_value(sharp_nt->reset_gpio, 0);
	return ret;
}

static int tm_otm1284a_panel_enable(struct drm_panel *panel)
{
	struct tm_otm1284a_panel *sharp_nt = to_tm_otm1284a_panel(panel);

	if (sharp_nt->enabled)
		return 0;

	if (sharp_nt->backlight) {
		sharp_nt->backlight->props.power = FB_BLANK_UNBLANK;
		backlight_update_status(sharp_nt->backlight);
	}

	sharp_nt->enabled = true;

	return 0;
}

static const struct drm_display_mode default_mode = {
	.clock = 66405,
	.hdisplay = 720,
	.hsync_start = 720 + 53,
	.hsync_end = 720 + 53 + 4,
	.htotal = 720 + 53 + 4 + 64,
	.vdisplay = 1280,
	.vsync_start = 1280 + 16,
	.vsync_end = 1280 + 16 + 4,
	.vtotal = 1280 + 16 + 4 + 16,
	.vrefresh = 60,
};

static int tm_otm1284a_panel_get_modes(struct drm_panel *panel)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(panel->drm, &default_mode);
	if (!mode) {
		dev_err(panel->drm->dev, "failed to add mode %ux%ux@%u\n",
				default_mode.hdisplay, default_mode.vdisplay,
				default_mode.vrefresh);
		return -ENOMEM;
	}

	drm_mode_set_name(mode);

	drm_mode_probed_add(panel->connector, mode);

	panel->connector->display_info.width_mm = 62;
	panel->connector->display_info.height_mm = 110;

	return 1;
}

static const struct drm_panel_funcs tm_otm1284a_panel_funcs = {
	.disable = tm_otm1284a_panel_disable,
	.unprepare = tm_otm1284a_panel_unprepare,
	.prepare = tm_otm1284a_panel_prepare,
	.enable = tm_otm1284a_panel_enable,
	.get_modes = tm_otm1284a_panel_get_modes,
};

static int tm_otm1284a_panel_add(struct tm_otm1284a_panel *sharp_nt)
{
	struct device *dev = &sharp_nt->dsi->dev;
	struct device_node *np = NULL;
	int ret;

	sharp_nt->mode = &default_mode;

	sharp_nt->supply = devm_regulator_get(dev, "avdd");
	if (IS_ERR(sharp_nt->supply))
		return PTR_ERR(sharp_nt->supply);

	sharp_nt->reset_gpio = devm_gpiod_get(dev, "reset");
	if (IS_ERR(sharp_nt->reset_gpio)) {
		dev_err(dev, "cannot get reset-gpios %ld\n",
			PTR_ERR(sharp_nt->reset_gpio));
		sharp_nt->reset_gpio = NULL;
	} else {
		gpiod_set_value(sharp_nt->reset_gpio, 0);
	}

	np = of_parse_phandle(dev->of_node, "backlight", 0);
	if (np) {
		sharp_nt->backlight = of_find_backlight_by_node(np);
		of_node_put(np);

		if (!sharp_nt->backlight)
			return -EPROBE_DEFER;
	}

	drm_panel_init(&sharp_nt->base);
	sharp_nt->base.funcs = &tm_otm1284a_panel_funcs;
	sharp_nt->base.dev = &sharp_nt->dsi->dev;

	ret = drm_panel_add(&sharp_nt->base);
	if (ret < 0)
		goto put_backlight;

	return 0;

put_backlight:
	if (sharp_nt->backlight)
		put_device(&sharp_nt->backlight->dev);

	return ret;
}

static void tm_otm1284a_panel_del(struct tm_otm1284a_panel *sharp_nt)
{
	if (sharp_nt->base.dev)
		drm_panel_remove(&sharp_nt->base);

	if (sharp_nt->backlight)
		put_device(&sharp_nt->backlight->dev);
}

static int tm_otm1284a_panel_probe(struct mipi_dsi_device *dsi)
{
	struct tm_otm1284a_panel *sharp_nt;
	int ret;

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO |
			MIPI_DSI_MODE_VIDEO_HSE |
			MIPI_DSI_CLOCK_NON_CONTINUOUS |
			MIPI_DSI_MODE_EOT_PACKET;

	sharp_nt = devm_kzalloc(&dsi->dev, sizeof(*sharp_nt), GFP_KERNEL);
	if (!sharp_nt)
		return -ENOMEM;

	mipi_dsi_set_drvdata(dsi, sharp_nt);

	sharp_nt->dsi = dsi;

	ret = tm_otm1284a_panel_add(sharp_nt);
	if (ret < 0)
		return ret;

	return mipi_dsi_attach(dsi);
}

static int tm_otm1284a_panel_remove(struct mipi_dsi_device *dsi)
{
	struct tm_otm1284a_panel *sharp_nt = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = tm_otm1284a_panel_disable(&sharp_nt->base);
	if (ret < 0)
		dev_err(&dsi->dev, "failed to disable panel: %d\n", ret);

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "failed to detach from DSI host: %d\n", ret);

	drm_panel_detach(&sharp_nt->base);
	tm_otm1284a_panel_del(sharp_nt);

	return 0;
}

static void tm_otm1284a_panel_shutdown(struct mipi_dsi_device *dsi)
{
	struct tm_otm1284a_panel *sharp_nt = mipi_dsi_get_drvdata(dsi);

	tm_otm1284a_panel_disable(&sharp_nt->base);
}

static const struct of_device_id sharp_nt_of_match[] = {
	{ .compatible = "sharp,ls043t1le01-qhd", },
	{ }
};
MODULE_DEVICE_TABLE(of, sharp_nt_of_match);

static struct mipi_dsi_driver tm_otm1284a_panel_driver = {
	.driver = {
		.name = "panel-sharp-ls043t1le01-qhd",
		.of_match_table = sharp_nt_of_match,
	},
	.probe = tm_otm1284a_panel_probe,
	.remove = tm_otm1284a_panel_remove,
	.shutdown = tm_otm1284a_panel_shutdown,
};
module_mipi_dsi_driver(tm_otm1284a_panel_driver);

MODULE_AUTHOR("tmp64 <tmp32x2@gmail.com>");
MODULE_AUTHOR("Werner Johansson <werner.johansson@sonymobile.com>");
MODULE_DESCRIPTION("TM otm1284a 720p video mode panel driver");
MODULE_LICENSE("GPL v2");
