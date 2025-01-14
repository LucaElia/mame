// license:BSD-3-Clause
// copyright-holders:Victor Trucco, Mike Balfour, Phil Stroffolino
/******************************************************************

Shark Attack
(C) 1980 PACIFIC NOVELTY MFG. INC.

Thief
(C) 1981 PACIFIC NOVELTY MFG. INC.

NATO Defense
(C) 1982 PACIFIC NOVELTY MFG. INC.

Credits:
    Shark Driver by Victor Trucco and Mike Balfour
    Driver for Thief and NATO Defense by Phil Stroffolino

- minor blitting glitches in playfield of Thief (XOR vs copy?)

- Nato Defense gfx ROMs may be hooked up wrong;
    see screenshots from flyers

******************************************************************/

#include "emu.h"
#include "thief.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "screen.h"
#include "speaker.h"



WRITE_LINE_MEMBER(thief_state::slam_w)
{
	/* SLAM switch causes an NMI if it's pressed */
	m_maincpu->set_input_line(INPUT_LINE_NMI, state ? CLEAR_LINE : ASSERT_LINE);
}

IRQ_CALLBACK_MEMBER(thief_state::iack)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0xff;
}

/**********************************************************/


/*  Following is an attempt to simulate the behavior of the
**  cassette tape used in several Pacific Novelty games.
**
**  It is a leaderless tape that is constructed so that it will
**  loop continuously.  The IO controller can start and stop the
**  tape player's motor, and enable/disable each of two audio
**  tracks.
*/

enum
{
	kTalkTrack, kCrashTrack
};

void thief_state::tape_set_audio( int track, int bOn )
{
	m_samples->set_volume(track, bOn ? 1.0 : 0.0 );
}

void thief_state::tape_set_motor( int bOn )
{
	if( bOn )
	{
		/* If talk track is not playing, start it. */
		if (! m_samples->playing( kTalkTrack ))
			m_samples->start( 0, kTalkTrack, true );

		/* Resume playback of talk track. */
		m_samples->pause( kTalkTrack, false);


		/* If crash track is not playing, start it. */
		if (! m_samples->playing( kCrashTrack ))
			m_samples->start( 1, kCrashTrack, true );

		/* Resume playback of crash track. */
		m_samples->pause( kCrashTrack, false);
	}
	else
	{
		/* Pause both the talk and crash tracks. */
		m_samples->pause( kTalkTrack, true );
		m_samples->pause( kCrashTrack, true );
	}
}

/***********************************************************/

void thief_state::thief_input_select_w(uint8_t data)
{
	m_input_select = data;
}

void thief_state::tape_control_w(uint8_t data)
{
	// avoid bogus coin counts after reset
	if (data == 0x00)
		return;

	// 7---32-0  not used
	// -6------  speaker right (crash track)
	// --5-----  tape motor
	// ---4----  speaker left (talk track)
	// ------1-  coin meter

	machine().bookkeeping().coin_counter_w(0, BIT(data, 1) ? 0 : 1);

	tape_set_audio(kTalkTrack, BIT(data, 4) ? 0 : 1);
	tape_set_motor(BIT(data, 5) ? 0 : 1);
	tape_set_audio(kCrashTrack, BIT(data, 6) ? 0 : 1);
}

uint8_t thief_state::thief_io_r()
{
	uint8_t data = 0xff;

	if (BIT(m_input_select, 0)) data &= ioport("DSW1")->read();
	if (BIT(m_input_select, 1)) data &= ioport("DSW2")->read();
	if (BIT(m_input_select, 2)) data &= ioport("P1")->read();
	if (BIT(m_input_select, 3)) data &= ioport("P2")->read();

	return data;
}

void thief_state::sharkatt_main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram();     /* 2114 */
	map(0xc000, 0xdfff).rw(FUNC(thief_state::thief_videoram_r), FUNC(thief_state::thief_videoram_w));   /* 4116 */
}

void thief_state::thief_main_map(address_map &map)
{
	map(0x0000, 0x0000).w(FUNC(thief_state::thief_blit_w));
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram();     /* 2114 */
	map(0xa000, 0xafff).rom();     /* NATO Defense diagnostic ROM */
	map(0xc000, 0xdfff).rw(FUNC(thief_state::thief_videoram_r), FUNC(thief_state::thief_videoram_w));   /* 4116 */
	map(0xe000, 0xe008).rw(FUNC(thief_state::thief_coprocessor_r), FUNC(thief_state::thief_coprocessor_w));
	map(0xe010, 0xe02f).rom();
	map(0xe080, 0xe0bf).rw(FUNC(thief_state::thief_context_ram_r), FUNC(thief_state::thief_context_ram_w));
	map(0xe0c0, 0xe0c0).w(FUNC(thief_state::thief_context_bank_w));
}


void thief_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).nopw(); /* watchdog */
	map(0x10, 0x10).w(FUNC(thief_state::thief_video_control_w));
	map(0x30, 0x33).mirror(0x0c).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x41).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x41, 0x41).r("ay1", FUNC(ay8910_device::data_r));
	map(0x42, 0x43).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x43, 0x43).r("ay2", FUNC(ay8910_device::data_r));
	map(0x50, 0x50).w(FUNC(thief_state::thief_color_plane_w));
	map(0x60, 0x6f).rw(m_tms, FUNC(tms9927_device::read), FUNC(tms9927_device::write));
	map(0x70, 0x7f).w(FUNC(thief_state::thief_color_map_w));
}


/**********************************************************/

static INPUT_PORTS_START( sharkatt )
	PORT_START("DSW1")  /* IN0 */
	PORT_DIPNAME( 0x7f, 0x7f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x7f, DEF_STR( 1C_1C ) ) // if any are set
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("DSW2")  /* IN1 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
//  PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("P1")    /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("P2")    /* IN3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, thief_state, slam_w)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( thief )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x000, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "7" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00|0x0c, "10K" )
	PORT_DIPSETTING(    0x01|0x0c, "20K" )
	PORT_DIPSETTING(    0x02|0x0c, "30K" )
	PORT_DIPSETTING(    0x03|0x0c, "40K" )
	PORT_DIPSETTING(    0x00|0x08, "10K 10K" )
	PORT_DIPSETTING(    0x01|0x08, "20K 20K" )
	PORT_DIPSETTING(    0x02|0x08, "30K 30K" )
	PORT_DIPSETTING(    0x03|0x08, "40K 40K" )
	PORT_DIPSETTING(    0x00,      DEF_STR( None ) )
	PORT_DIPNAME( 0xf0, 0x00, "Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x70, "Display Options" )
	PORT_DIPSETTING(    0x80|0x00, "Burn-in Test" )
	PORT_DIPSETTING(    0x80|0x10, "Color Bar Test" )
	PORT_DIPSETTING(    0x80|0x20, "Cross Hatch" )
	PORT_DIPSETTING(    0x80|0x30, "Color Map" )
	PORT_DIPSETTING(    0x80|0x40, "VIDSEL Test" )
	PORT_DIPSETTING(    0x80|0x50, "VIDBIT Test" )
	PORT_DIPSETTING(    0x80|0x60, "I/O Board Test" )
	PORT_DIPSETTING(    0x80|0x70, "Reserved" )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, thief_state, slam_w)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( natodef )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x000, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "7" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "Add a Coin?" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0b, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "10K" )
	PORT_DIPSETTING(    0x09, "20K" )
	PORT_DIPSETTING(    0x0a, "30K" )
	PORT_DIPSETTING(    0x0b, "40K" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xf0, 0x00, "Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x70, "Display Options" )
	PORT_DIPSETTING(    0x80|0x00, "Burn-in Test" )
	PORT_DIPSETTING(    0x80|0x10, "Color Bar Test" )
	PORT_DIPSETTING(    0x80|0x20, "Cross Hatch" )
	PORT_DIPSETTING(    0x80|0x30, "Color Map" )
	PORT_DIPSETTING(    0x80|0x40, "VIDSEL Test" )
	PORT_DIPSETTING(    0x80|0x50, "VIDBIT Test" )
	PORT_DIPSETTING(    0x80|0x60, "I/O Board Test" )
	PORT_DIPSETTING(    0x80|0x70, "Reserved" )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, thief_state, slam_w)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***********************************************************/

static const char *const sharkatt_sample_names[] =
{
	"*sharkatt",
	"talk",
	"crash",
	nullptr   /* end of array */
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static const char *const thief_sample_names[] =
{
	"*thief",
	"talk",
	"crash",
	nullptr   /* end of array */
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static const char *const natodef_sample_names[] =
{
	"*natodef",
	"talk",
	"crash",
	nullptr   /* end of array */
};


void thief_state::thief(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(8'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &thief_state::thief_main_map);
	m_maincpu->set_addrmap(AS_IO, &thief_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(thief_state::iack));

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.out_pa_callback().set(FUNC(thief_state::thief_input_select_w));
	ppi.in_pb_callback().set(FUNC(thief_state::thief_io_r));
	ppi.out_pc_callback().set(FUNC(thief_state::tape_control_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(20'000'000)/4, 320, 0, 256, 272, 0, 256);
	m_screen->set_screen_update(FUNC(thief_state::screen_update_thief));
	m_screen->set_palette(m_palette);

	TMS9927(config, m_tms, XTAL(20'000'000)/4/8);
	m_tms->set_char_width(8);
	m_tms->vsyn_callback().set_inputline("maincpu", 0, ASSERT_LINE);

	PALETTE(config, m_palette).set_entries(16);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay1", XTAL(8'000'000)/2/4).add_route(ALL_OUTPUTS, "mono", 0.50);

	AY8910(config, "ay2", XTAL(8'000'000)/2/4).add_route(ALL_OUTPUTS, "mono", 0.50);

	SAMPLES(config, m_samples);
	m_samples->set_channels(2);
	m_samples->set_samples_names(thief_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void thief_state::sharkatt(machine_config &config)
{
	thief(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &thief_state::sharkatt_main_map);

	m_screen->set_visarea(0*8, 32*8-1, 0*8, 24*8-1);

	m_samples->set_samples_names(sharkatt_sample_names);
}

void thief_state::natodef(machine_config &config)
{
	thief(config);
	m_samples->set_samples_names(natodef_sample_names);
}


/**********************************************************/

ROM_START( sharkatt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sharkatt.0",   0x0000, 0x800, CRC(c71505e9) SHA1(068c92e9d797918f281fa509f3c86578b3f0de3a) )
	ROM_LOAD( "sharkatt.1",   0x0800, 0x800, CRC(3e3abf70) SHA1(ef69e72db583a22093a3c32ba437a6eaef4b132a) )
	ROM_LOAD( "sharkatt.2",   0x1000, 0x800, CRC(96ded944) SHA1(e60db225111423b0a481e85fe38a85c3ea844351) )
	ROM_LOAD( "sharkatt.3",   0x1800, 0x800, CRC(007283ae) SHA1(1c311c03729573a4aa6656972e193024364a2f2a) )
	ROM_LOAD( "sharkatt.4a",  0x2000, 0x800, CRC(5cb114a7) SHA1(4240fe1bcc1501b22da133dfb42746b6752b3aea) )
	ROM_LOAD( "sharkatt.5",   0x2800, 0x800, CRC(1d88aaad) SHA1(c81f6d75d88af067f33ff84c417908c450e9e280) )
	ROM_LOAD( "sharkatt.6",   0x3000, 0x800, CRC(c164bad4) SHA1(d72e896bd4b5b0863f2ef8e621e78dd324f9d2c8) )
	ROM_LOAD( "sharkatt.7",   0x3800, 0x800, CRC(d78c4b8b) SHA1(c0371dccfb997331b31893b54fe3c749632dc171) )
	ROM_LOAD( "sharkatt.8",   0x4000, 0x800, CRC(5958476a) SHA1(2063a9721a6eec5049191c69089c3d8cc3064b69) )
	ROM_LOAD( "sharkatt.9",   0x4800, 0x800, CRC(4915eb37) SHA1(56ec2745241afd76aeaa30fb0010cedfd55f307b) )
	ROM_LOAD( "sharkatt.10",  0x5000, 0x800, CRC(9d07cb68) SHA1(528a42e8e7696452bb9d376222f3cbfcb238c01d) )
	ROM_LOAD( "sharkatt.11",  0x5800, 0x800, CRC(21edc962) SHA1(8af23e471b6eb11fc55f331ec97a94e2e6c8be80) )
	ROM_LOAD( "sharkatt.12a", 0x6000, 0x800, CRC(5dd8785a) SHA1(4eaceb781271757c4f4f6f9a4647d394d1912d72) )
ROM_END

ROM_START( thief )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code */
	ROM_LOAD( "t8a0ah0a",   0x0000, 0x1000, CRC(edbbf71c) SHA1(9f13841c54fbe5449280c24954a45517014a834e) )
	ROM_LOAD( "t2662h2",    0x1000, 0x1000, CRC(85b4f6ff) SHA1(8e007bfff2f27809e7a9881bc3b2587bf35cff6d) )
	ROM_LOAD( "tc162h4",    0x2000, 0x1000, CRC(70478a82) SHA1(547bad88a44c63657bf8f65f2877ab1323515521) )
	ROM_LOAD( "t0cb4h6",    0x3000, 0x1000, CRC(29de0425) SHA1(6614f3ee314ebf2a6469481e8c69c32a93fa8eb5) )
	ROM_LOAD( "tc707h8",    0x4000, 0x1000, CRC(ea8dd847) SHA1(eab24621abe3735902f03463ee536a0cbfeb7407) )
	ROM_LOAD( "t857bh10",   0x5000, 0x1000, CRC(403c33b7) SHA1(d1422e74c9ecdadbc238b155f853294f6bb83992) )
	ROM_LOAD( "t606bh12",   0x6000, 0x1000, CRC(4ca2748b) SHA1(07df2fac63471d716923f859105421e22e5e970e) )
	ROM_LOAD( "tae4bh14",   0x7000, 0x1000, CRC(22e7dcc3) SHA1(fd4302688905bbd47dfdc1d7cdb55212a5e99f81) ) /* diagnostics ROM */

	ROM_REGION( 0x400, "cpu1", 0 ) /* coprocessor */
	ROM_LOAD( "b8",         0x000, 0x0200, CRC(fe865b2a) SHA1(b29144b05cb2846ea9c868ebf843d74d94c7bcc6) )
	/* B8 is a function dispatch table for the coprocessor (unused) */
	ROM_LOAD( "c8",         0x200, 0x0200, CRC(7ed5c923) SHA1(35757d50bfa9ea3cf916576a148064a0f9be8732) )
	/* C8 is mapped (banked) in CPU1's address space; it contains Z80 code */

	ROM_REGION( 0x6000, "gfx1", 0 ) /* image ROMs for coprocessor */
	ROM_LOAD16_BYTE( "t079ahd4" ,  0x0001, 0x1000, CRC(928bd8ef) SHA1(3a2de005176ef012c0411d7752a69c03fb165b28) )
	ROM_LOAD16_BYTE( "tdda7hh4" ,  0x0000, 0x1000, CRC(b48f0862) SHA1(c62ccf407e819fe7fa94a4353a17da47b91f0606) )
	/* next 0x4000 bytes are unmapped (used by Nato Defense) */
ROM_END

ROM_START( natodef )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code */
	ROM_LOAD( "natodef.cp0",    0x0000, 0x1000, CRC(8397c787) SHA1(5957613f1ace7dc4612f28f6fba3a7374be905ac) )
	ROM_LOAD( "natodef.cp2",    0x1000, 0x1000, CRC(8cfbf26f) SHA1(a15f0d5d82cd96b80ee91dc91858b660c5895f34) )
	ROM_LOAD( "natodef.cp4",    0x2000, 0x1000, CRC(b4c90fb2) SHA1(3ff4691415433863bfe74d51b9f3aa428f3bf88f) )
	ROM_LOAD( "natodef.cp6",    0x3000, 0x1000, CRC(c6d0d35e) SHA1(d4f34b4930be6dc67d77af691d14ee3b797ec29d) )
	ROM_LOAD( "natodef.cp8",    0x4000, 0x1000, CRC(e4b6c21e) SHA1(cfdae66494bc2cc9ee414b9adcf8257b7c69bb40) )
	ROM_LOAD( "natodef.cpa",    0x5000, 0x1000, CRC(888ecd42) SHA1(5af638d7e299046d5803d2764bf42ea44a80374c) )
	ROM_LOAD( "natodef.cpc",    0x6000, 0x1000, CRC(cf713bc9) SHA1(0687755a6cfd76a920c210bf11530ef4c59d92b0) )
	ROM_LOAD( "natodef.cpe",    0x7000, 0x1000, CRC(4eef6bf4) SHA1(ab094198ea4d2267194ace5d382abb78d568983a) )
	ROM_LOAD( "natodef.cp5",    0xa000, 0x1000, CRC(65c3601b) SHA1(c7bf31e6cb781405b3665b3aa93644ed57616256) )  /* diagnostics ROM */

	ROM_REGION( 0x400, "cpu1", 0 ) /* coprocessor */
	ROM_LOAD( "b8",         0x000, 0x0200, CRC(fe865b2a) SHA1(b29144b05cb2846ea9c868ebf843d74d94c7bcc6) )
	ROM_LOAD( "c8",         0x200, 0x0200, CRC(7ed5c923) SHA1(35757d50bfa9ea3cf916576a148064a0f9be8732) )
	/* C8 is mapped (banked) in CPU1's address space; it contains Z80 code */

	ROM_REGION( 0x6000, "gfx1", 0 ) /* image ROMs for coprocessor */
	ROM_LOAD16_BYTE( "natodef.o4",  0x0001, 0x1000, CRC(39a868f8) SHA1(870795f18cd8f831b714b809a380e30b5d323a5f) )
	ROM_LOAD16_BYTE( "natodef.e1",  0x0000, 0x1000, CRC(b6d1623d) SHA1(0aa15db0e1459a6cc7d2a5bc8e588fd514b71d85) )
	ROM_LOAD16_BYTE( "natodef.o2",  0x2001, 0x1000, CRC(77cc9cfd) SHA1(1bbed3cb834b844fb2d9d48a3a142edaeb33ccc6) )
	ROM_LOAD16_BYTE( "natodef.e3",  0x2000, 0x1000, CRC(5302410d) SHA1(e166c151d948f474c134802e3f891982bf370596) )
	ROM_LOAD16_BYTE( "natodef.o3",  0x4001, 0x1000, CRC(b217909a) SHA1(a26eb5bf2c92d79a75376deb6278710426b34cc5) )
	ROM_LOAD16_BYTE( "natodef.e2",  0x4000, 0x1000, CRC(886c3f05) SHA1(306c8621455d2d6b7b2f545500b27e56a7159a1b) )
ROM_END

ROM_START( natodefa )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code */
	ROM_LOAD( "natodef.cp0",    0x0000, 0x1000, CRC(8397c787) SHA1(5957613f1ace7dc4612f28f6fba3a7374be905ac) )
	ROM_LOAD( "natodef.cp2",    0x1000, 0x1000, CRC(8cfbf26f) SHA1(a15f0d5d82cd96b80ee91dc91858b660c5895f34) )
	ROM_LOAD( "natodef.cp4",    0x2000, 0x1000, CRC(b4c90fb2) SHA1(3ff4691415433863bfe74d51b9f3aa428f3bf88f) )
	ROM_LOAD( "natodef.cp6",    0x3000, 0x1000, CRC(c6d0d35e) SHA1(d4f34b4930be6dc67d77af691d14ee3b797ec29d) )
	ROM_LOAD( "natodef.cp8",    0x4000, 0x1000, CRC(e4b6c21e) SHA1(cfdae66494bc2cc9ee414b9adcf8257b7c69bb40) )
	ROM_LOAD( "natodef.cpa",    0x5000, 0x1000, CRC(888ecd42) SHA1(5af638d7e299046d5803d2764bf42ea44a80374c) )
	ROM_LOAD( "natodef.cpc",    0x6000, 0x1000, CRC(cf713bc9) SHA1(0687755a6cfd76a920c210bf11530ef4c59d92b0) )
	ROM_LOAD( "natodef.cpe",    0x7000, 0x1000, CRC(4eef6bf4) SHA1(ab094198ea4d2267194ace5d382abb78d568983a) )
	ROM_LOAD( "natodef.cp5",    0xa000, 0x1000, CRC(65c3601b) SHA1(c7bf31e6cb781405b3665b3aa93644ed57616256) )  /* diagnostics ROM */

	ROM_REGION( 0x400, "cpu1", 0 ) /* coprocessor */
	ROM_LOAD( "b8",         0x000, 0x0200, CRC(fe865b2a) SHA1(b29144b05cb2846ea9c868ebf843d74d94c7bcc6) )
	ROM_LOAD( "c8",         0x200, 0x0200, CRC(7ed5c923) SHA1(35757d50bfa9ea3cf916576a148064a0f9be8732) )
	/* C8 is mapped (banked) in CPU1's address space; it contains Z80 code */

	ROM_REGION( 0x6000, "gfx1", 0 ) /* image ROMs for coprocessor */
	ROM_LOAD16_BYTE( "natodef.o4",  0x0001, 0x1000, CRC(39a868f8) SHA1(870795f18cd8f831b714b809a380e30b5d323a5f) )
	ROM_LOAD16_BYTE( "natodef.e1",  0x0000, 0x1000, CRC(b6d1623d) SHA1(0aa15db0e1459a6cc7d2a5bc8e588fd514b71d85) )
	ROM_LOAD16_BYTE( "natodef.o3",  0x2001, 0x1000, CRC(b217909a) SHA1(a26eb5bf2c92d79a75376deb6278710426b34cc5) ) /* same ROMs as natodef, */
	ROM_LOAD16_BYTE( "natodef.e2",  0x2000, 0x1000, CRC(886c3f05) SHA1(306c8621455d2d6b7b2f545500b27e56a7159a1b) ) /* but in a different */
	ROM_LOAD16_BYTE( "natodef.o2",  0x4001, 0x1000, CRC(77cc9cfd) SHA1(1bbed3cb834b844fb2d9d48a3a142edaeb33ccc6) ) /* order to give */
	ROM_LOAD16_BYTE( "natodef.e3",  0x4000, 0x1000, CRC(5302410d) SHA1(e166c151d948f474c134802e3f891982bf370596) ) /* different mazes */
ROM_END


void thief_state::init_thief()
{
	uint8_t *dest = memregion("maincpu")->base();
	const uint8_t *source = memregion("cpu1")->base();

	/* C8 is mapped (banked) in CPU1's address space; it contains Z80 code */
	memcpy(&dest[0xe010], &source[0x290], 0x20);
}


GAME( 1980, sharkatt, 0,       sharkatt, sharkatt, thief_state, empty_init, ROT0, "Pacific Novelty", "Shark Attack",                    0 )
GAME( 1981, thief,    0,       thief,    thief,    thief_state, init_thief, ROT0, "Pacific Novelty", "Thief",                           0 )
GAME( 1982, natodef,  0,       natodef,  natodef,  thief_state, init_thief, ROT0, "Pacific Novelty", "NATO Defense" ,                   0 )
GAME( 1982, natodefa, natodef, natodef,  natodef,  thief_state, init_thief, ROT0, "Pacific Novelty", "NATO Defense (alternate mazes)" , 0 )
