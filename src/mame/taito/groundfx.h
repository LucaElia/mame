// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
#ifndef MAME_INCLUDES_GROUNDFX_H
#define MAME_INCLUDES_GROUNDFX_H

#pragma once

#include "tc0100scn.h"
#include "tc0480scp.h"
#include "emupal.h"

struct gfx_tempsprite
{
	u8 gfx = 0U;
	u32 code = 0U, color = 0U;
	bool flipx = false, flipy = false;
	int x = 0, y = 0;
	int zoomx = 0, zoomy = 0;
	int pri = 0;
};

class groundfx_state : public driver_device
{
public:
	groundfx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_ram(*this, "ram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_tc0620scc(*this, "tc0620scc"),
		m_tc0480scp(*this, "tc0480scp"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritemap(*this, "spritemap")
	{ }

	void groundfx(machine_config &config);
	void init_groundfx();

protected:
	virtual void video_start() override;

private:
	required_shared_ptr<u32> m_ram;
	required_shared_ptr<u32> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<tc0620scc_device> m_tc0620scc;
	required_device<tc0480scp_device> m_tc0480scp;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_region_ptr<u16> m_spritemap;

	u16 m_frame_counter = 0U;
	u16 m_port_sel = 0U;
	std::unique_ptr<gfx_tempsprite[]> m_spritelist{};
	u16 m_rotate_ctrl[8]{};
	rectangle m_hack_cliprect{};

	void rotate_control_w(offs_t offset, u16 data);
	void motor_control_w(u32 data);
	u32 irq_speedup_r();
	DECLARE_READ_LINE_MEMBER(frame_counter_r);
	void coin_word_w(u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,int do_hack,int x_offs,int y_offs);

	void groundfx_map(address_map &map);
};

#endif // MAME_INCLUDES_GROUNDFX_H
