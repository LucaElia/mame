// license:BSD-3-Clause
// copyright-holders:Brad Oliver
#ifndef MAME_TECHNOS_MATMANIA_H
#define MAME_TECHNOS_MATMANIA_H

#pragma once

#include "taito68705.h"

#include "machine/gen_latch.h"

#include "emupal.h"
#include "screen.h"

class matmania_state : public driver_device
{
public:
	matmania_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_videoram3(*this, "videoram3"),
		m_colorram(*this, "colorram"),
		m_colorram2(*this, "colorram2"),
		m_colorram3(*this, "colorram3"),
		m_scroll(*this, "scroll"),
		m_pageselect(*this, "pageselect"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void matmania(machine_config &config);
	void maniach(machine_config &config);

protected:
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_videoram3;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_colorram2;
	required_shared_ptr<uint8_t> m_colorram3;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_pageselect;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_paletteram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<taito68705_mcu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* video-related */
	std::unique_ptr<bitmap_ind16> m_tmpbitmap;
	std::unique_ptr<bitmap_ind16> m_tmpbitmap2;

	uint8_t maniach_mcu_status_r();
	void matmania_sh_command_w(uint8_t data);
	void maniach_sh_command_w(uint8_t data);
	void matmania_paletteram_w(offs_t offset, uint8_t data);
	void matmania_palette(palette_device &palette) const;
	uint32_t screen_update_matmania(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_maniach(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void maniach_map(address_map &map);
	void maniach_sound_map(address_map &map);
	void matmania_map(address_map &map);
	void matmania_sound_map(address_map &map);
};

#endif // MAME_TECHNOS_MATMANIA_H
