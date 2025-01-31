// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Technos Mysterious Stones hardware

    driver by Nicola Salmoria

    Notes:
        * The subtitle of the two sets is slightly different:
          "Dr. John's Adventure" vs. "Dr. Kick in Adventure".
          The Dr John's is a bug fix. See the routine at 4376/4384 for example.
          The old set thrashes the Y register, the new one saves in on
          the stack. The newer set also resets the audio chips more often.

***************************************************************************/

#include "emu.h"
#include "mystston.h"

#include "cpu/m6502/m6502.h"
#include "speaker.h"


/*************************************
 *
 *  Interrupt system
 *
 *************************************/

void mystston_state::on_scanline_interrupt()
{
	m_maincpu->set_input_line(0, ASSERT_LINE);
}


void mystston_state::irq_clear_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}



/*************************************
 *
 *  Coin handling
 *
 *************************************/

INPUT_CHANGED_MEMBER(mystston_state::coin_inserted)
{
	// coin insertion causes an NMI
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}



/*************************************
 *
 *  AY-8910 memory interface
 *
 *************************************/

void mystston_state::ay8910_select_w(uint8_t data)
{
	// bit 5 goes to 8910 #0 BDIR pin
	if (((*m_ay8910_select & 0x20) == 0x20) && ((data & 0x20) == 0x00))
	{
		// bit 4 goes to the 8910 #0 BC1 pin
		m_ay8910[0]->data_address_w(*m_ay8910_select >> 4, *m_ay8910_data);
	}

	// bit 7 goes to 8910 #1 BDIR pin
	if (((*m_ay8910_select & 0x80) == 0x80) && ((data & 0x80) == 0x00))
	{
		// bit 6 goes to the 8910 #1 BC1 pin
		m_ay8910[1]->data_address_w(*m_ay8910_select >> 6, *m_ay8910_data);
	}

	*m_ay8910_select = data;
}



/*************************************
 *
 *  Memory map
 *
 *************************************/

void mystston_state::main_map(address_map &map)
{
	map(0x0000, 0x077f).ram();
	map(0x0780, 0x07df).ram().share(m_spriteram);
	map(0x07e0, 0x0fff).ram();
	map(0x1000, 0x17ff).ram().share(m_fg_videoram);
	map(0x1800, 0x1fff).ram().share(m_bg_videoram);
	map(0x2000, 0x2000).mirror(0x1f8f).portr("IN0").w(FUNC(mystston_state::video_control_w)).share(m_video_control);
	map(0x2010, 0x2010).mirror(0x1f8f).portr("IN1").w(FUNC(mystston_state::irq_clear_w));
	map(0x2020, 0x2020).mirror(0x1f8f).portr("DSW0").writeonly().share(m_scroll);
	map(0x2030, 0x2030).mirror(0x1f8f).portr("DSW1").writeonly().share(m_ay8910_data);
	map(0x2040, 0x2040).mirror(0x1f8f).nopr().w(FUNC(mystston_state::ay8910_select_w)).share(m_ay8910_select);
	map(0x2050, 0x2050).mirror(0x1f8f).noprw();
	map(0x2060, 0x207f).mirror(0x1f80).ram().share(m_paletteram);
	map(0x4000, 0xffff).rom();
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mystston )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, mystston_state, coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, mystston_state, coin_inserted, 0)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW0")
	PORT_DIPNAME(0x01, 0x01, DEF_STR( Lives ) )     PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(   0x01, "3" )
	PORT_DIPSETTING(   0x00, "5" )
	PORT_DIPNAME(0x02, 0x02, DEF_STR( Difficulty ) )    PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(   0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Hard ) )
	PORT_DIPNAME(0x04, 0x00, DEF_STR( Demo_Sounds ) )   PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(   0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        // Listed as "Unused"

	PORT_START("DSW1")
	PORT_DIPNAME(0x03, 0x03, DEF_STR( Coin_A ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(   0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME(0x0c, 0x0c, DEF_STR( Coin_B ) )        PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(   0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(   0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(   0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )        // Listed as "Unused"
	PORT_DIPNAME(0x20, 0x00, DEF_STR( Flip_Screen ) )   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x20, DEF_STR( On ) )
	PORT_DIPNAME(0x40, 0x00, DEF_STR( Cabinet ) )       PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(   0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(   0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( myststonoi )
	PORT_INCLUDE(mystston)

	PORT_MODIFY("DSW0")
	PORT_DIPNAME(0x01, 0x01, DEF_STR( Lives ) )     PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(   0x01, "2" )
	PORT_DIPSETTING(   0x00, "3" )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mystston_state::mystston(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, MASTER_CLOCK / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &mystston_state::main_map);

	// video hardware
	video(config);

	// audio hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay8910[0], MASTER_CLOCK / 8).add_route(ALL_OUTPUTS, "mono", 0.30);

	AY8910(config, m_ay8910[1], MASTER_CLOCK / 8).add_route(ALL_OUTPUTS, "mono", 0.30);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( mystston )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom6.bin",     0x4000, 0x2000, CRC(7bd9c6cd) SHA1(4d14edc783ba1a6c01d2fb9ea29ec85b8fec3c3b) )
	ROM_LOAD( "rom5.bin",     0x6000, 0x2000, CRC(a83f04a6) SHA1(d8cdf310511c1fef4fbde80ef2161fda00f965d7) )
	ROM_LOAD( "rom4.bin",     0x8000, 0x2000, CRC(46c73714) SHA1(5b9ac3a35aeeea6a0cd2d838c144925d83b36a7f) )
	ROM_LOAD( "rom3.bin",     0xa000, 0x2000, CRC(34f8b8a3) SHA1(a270f6665a9f76f97ac02201d51fe2817e6e8f22) )
	ROM_LOAD( "rom2.bin",     0xc000, 0x2000, CRC(bfd22cfc) SHA1(137cd61c8b1e997e7e50edd57f1671031d8e3ac5) )
	ROM_LOAD( "rom1.bin",     0xe000, 0x2000, CRC(fb163e38) SHA1(d6f02e90bfd9badd7751bc0a87fdfdd1d0a7e202) )

	ROM_REGION( 0x0c000, "gfx1", 0 )
	ROM_LOAD( "ms6",          0x00000, 0x2000, CRC(85c83806) SHA1(cdfed6c224754e8f79b154533b06b7de4a44b4d3) )
	ROM_LOAD( "ms9",          0x02000, 0x2000, CRC(b146c6ab) SHA1(712c0c17780f222be5c8b09185a22e900ab23944) )
	ROM_LOAD( "ms7",          0x04000, 0x2000, CRC(d025f84d) SHA1(eaaaa0bde3db850098d04a0af85993026e503fc5) )
	ROM_LOAD( "ms10",         0x06000, 0x2000, CRC(d85015b5) SHA1(f4afab248dfde354650e59fadd5ab9616b04dac1) )
	ROM_LOAD( "ms8",          0x08000, 0x2000, CRC(53765d89) SHA1(c8bfc311123b076dccae9f7e3b95460bf9fc843d) )
	ROM_LOAD( "ms11",         0x0a000, 0x2000, CRC(919ee527) SHA1(609ee854ab3a4fdbf3404a68a4a657b85250f742) )

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "ms12",         0x00000, 0x2000, CRC(72d8331d) SHA1(f0a3bc6c9d9966f169f4721c2453f7ee210f0feb) )
	ROM_LOAD( "ms13",         0x02000, 0x2000, CRC(845a1f9b) SHA1(aa2eabd2a5e89e150b5d2fb3d88f91902e5ebb48) )
	ROM_LOAD( "ms14",         0x04000, 0x2000, CRC(822874b0) SHA1(9376d48045bf67df91d103effd1d08bd8debad26) )
	ROM_LOAD( "ms15",         0x06000, 0x2000, CRC(4594e53c) SHA1(a011a5269a9b0ca7a964181efe8413d5637c34f4) )
	ROM_LOAD( "ms16",         0x08000, 0x2000, CRC(2f470b0f) SHA1(79b50a7d113fed4669361c5f6c60ec96c94344c6) )
	ROM_LOAD( "ms17",         0x0a000, 0x2000, CRC(38966d1b) SHA1(89e3e54d3298cefeb35922d2292e3e7b8e995871) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "ic61",         0x0000, 0x0020, CRC(e802d6cf) SHA1(233ceb9e3a91939e1925766a696bc65ab0dffa50) )
ROM_END

/*
actual label format is:
--------------
|       T.M. |
| MYSTERIOUS |
|   STONES   |
|   BB00-    |
|  @ 1984    |
| DATA EAST  |
--------------
*/
ROM_START( myststono ) // TA-0010-P1-1 + TA-0010-P2-1 PCBs
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bb00.ic102", 0x4000, 0x2000, CRC(6dacc05f) SHA1(43054199901639516205c7ea145462d0abea8fb1) )
	ROM_LOAD( "bw01.ic90",  0x6000, 0x2000, CRC(a3546df7) SHA1(89c0349885a9369406a1121cd3db28963b25f2e6) )
	ROM_LOAD( "bw02.ic76",  0x8000, 0x2000, CRC(43bc6182) SHA1(dc36c10eee20009922e89d9bfdf6c2f6ffb881ce) )
	ROM_LOAD( "bw03.ic75",  0xa000, 0x2000, CRC(9322222b) SHA1(25192ac9e8e66cd2bc21c66c690c57c6b9836f2d) )
	ROM_LOAD( "bw04.ic62",  0xc000, 0x2000, CRC(47cefe9b) SHA1(49422b664b1322373a9cd3cb2907f8f5492faf87) )
	ROM_LOAD( "bw05.ic61",  0xe000, 0x2000, CRC(b37ae12b) SHA1(55ee1193088145c85adddd377d9e5ee58aca922f) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "bw06.ic105", 0x00000, 0x2000, CRC(85c83806) SHA1(cdfed6c224754e8f79b154533b06b7de4a44b4d3) )
	ROM_LOAD( "bw09.ic93",  0x02000, 0x2000, CRC(b146c6ab) SHA1(712c0c17780f222be5c8b09185a22e900ab23944) )
	ROM_LOAD( "bw07.ic107", 0x04000, 0x2000, CRC(d025f84d) SHA1(eaaaa0bde3db850098d04a0af85993026e503fc5) )
	ROM_LOAD( "bw10.ic95",  0x06000, 0x2000, CRC(d85015b5) SHA1(f4afab248dfde354650e59fadd5ab9616b04dac1) )
	ROM_LOAD( "bw08.ic109", 0x08000, 0x2000, CRC(53765d89) SHA1(c8bfc311123b076dccae9f7e3b95460bf9fc843d) )
	ROM_LOAD( "bw11.ic97",  0x0a000, 0x2000, CRC(919ee527) SHA1(609ee854ab3a4fdbf3404a68a4a657b85250f742) )

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "bw12.ic15", 0x00000, 0x2000, CRC(72d8331d) SHA1(f0a3bc6c9d9966f169f4721c2453f7ee210f0feb) )
	ROM_LOAD( "bw13.ic20", 0x02000, 0x2000, CRC(845a1f9b) SHA1(aa2eabd2a5e89e150b5d2fb3d88f91902e5ebb48) )
	ROM_LOAD( "bw14.ic24", 0x04000, 0x2000, CRC(822874b0) SHA1(9376d48045bf67df91d103effd1d08bd8debad26) )
	ROM_LOAD( "bw15.ic29", 0x06000, 0x2000, CRC(4594e53c) SHA1(a011a5269a9b0ca7a964181efe8413d5637c34f4) )
	ROM_LOAD( "bw16.ic34", 0x08000, 0x2000, CRC(2f470b0f) SHA1(79b50a7d113fed4669361c5f6c60ec96c94344c6) )
	ROM_LOAD( "bw17.ic38", 0x0a000, 0x2000, CRC(38966d1b) SHA1(89e3e54d3298cefeb35922d2292e3e7b8e995871) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "hlo.ic61", 0x0000, 0x0020, CRC(e802d6cf) SHA1(233ceb9e3a91939e1925766a696bc65ab0dffa50) )
ROM_END

// looks like Itisa made a (very) minor mod to the ROMs when producing the PCBs
ROM_START( myststonoi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "14.bin",          0x4000, 0x2000, CRC(78bf2a58) SHA1(92e61041acad3293a103d957507b091321eed8f1) ) // 2 bytes changed in here (values for the lives' dip-switch)
	ROM_LOAD( "13.bin",          0x6000, 0x2000, CRC(a3546df7) SHA1(89c0349885a9369406a1121cd3db28963b25f2e6) )
	ROM_LOAD( "12.bin",          0x8000, 0x2000, CRC(43bc6182) SHA1(dc36c10eee20009922e89d9bfdf6c2f6ffb881ce) )
	ROM_LOAD( "11.bin",          0xa000, 0x2000, CRC(9322222b) SHA1(25192ac9e8e66cd2bc21c66c690c57c6b9836f2d) )
	ROM_LOAD( "8.bin",           0xc000, 0x2000, CRC(47cefe9b) SHA1(49422b664b1322373a9cd3cb2907f8f5492faf87) )
	ROM_LOAD( "7.bin",           0xe000, 0x2000, CRC(b37ae12b) SHA1(55ee1193088145c85adddd377d9e5ee58aca922f) )

	ROM_REGION( 0x0c000, "gfx1", 0 )
	ROM_LOAD( "18.bin",          0x00000, 0x2000, CRC(85c83806) SHA1(cdfed6c224754e8f79b154533b06b7de4a44b4d3) )
	ROM_LOAD( "15.bin",          0x02000, 0x2000, CRC(b146c6ab) SHA1(712c0c17780f222be5c8b09185a22e900ab23944) )
	ROM_LOAD( "19.bin",          0x04000, 0x2000, CRC(d025f84d) SHA1(eaaaa0bde3db850098d04a0af85993026e503fc5) )
	ROM_LOAD( "16.bin",          0x06000, 0x2000, CRC(d85015b5) SHA1(f4afab248dfde354650e59fadd5ab9616b04dac1) )
	ROM_LOAD( "20.bin",          0x08000, 0x2000, CRC(53765d89) SHA1(c8bfc311123b076dccae9f7e3b95460bf9fc843d) )
	ROM_LOAD( "17.bin",          0x0a000, 0x2000, CRC(919ee527) SHA1(609ee854ab3a4fdbf3404a68a4a657b85250f742) )

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "1.bin",         0x00000, 0x2000, CRC(72d8331d) SHA1(f0a3bc6c9d9966f169f4721c2453f7ee210f0feb) )
	ROM_LOAD( "2.bin",         0x02000, 0x2000, CRC(845a1f9b) SHA1(aa2eabd2a5e89e150b5d2fb3d88f91902e5ebb48) )
	ROM_LOAD( "3.bin",         0x04000, 0x2000, CRC(822874b0) SHA1(9376d48045bf67df91d103effd1d08bd8debad26) )
	ROM_LOAD( "4.bin",         0x06000, 0x2000, CRC(4594e53c) SHA1(a011a5269a9b0ca7a964181efe8413d5637c34f4) )
	ROM_LOAD( "5.bin",         0x08000, 0x2000, CRC(2f470b0f) SHA1(79b50a7d113fed4669361c5f6c60ec96c94344c6) )
	ROM_LOAD( "6.bin",         0x0a000, 0x2000, CRC(38966d1b) SHA1(89e3e54d3298cefeb35922d2292e3e7b8e995871) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.bin",         0x0000, 0x0020, CRC(e802d6cf) SHA1(233ceb9e3a91939e1925766a696bc65ab0dffa50) )

	ROM_REGION( 0x0104, "pals", 0 ) // not verified if these were protected
	ROM_LOAD( "pal10l8.bin",         0x0000, 0x002c, CRC(2d4d034c) SHA1(1d43f21d4522c71da507c1ded1b2d3bfa0fe043b) )
	ROM_LOAD( "pal16r4-1.bin",       0x0000, 0x0104, CRC(c57555d0) SHA1(c1cda869de8457b9f8ca4f41f0ed49916110ff2e) )
	ROM_LOAD( "pal16r4-2.bin",       0x0000, 0x0104, CRC(c57555d0) SHA1(c1cda869de8457b9f8ca4f41f0ed49916110ff2e) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1984, mystston,   0,        mystston, mystston,   mystston_state, empty_init, ROT270, "Technos Japan", "Mysterious Stones - Dr. John's Adventure",              MACHINE_SUPPORTS_SAVE )
GAME( 1984, myststono,  mystston, mystston, mystston,   mystston_state, empty_init, ROT270, "Technos Japan", "Mysterious Stones - Dr. Kick in Adventure",             MACHINE_SUPPORTS_SAVE )
GAME( 1984, myststonoi, mystston, mystston, myststonoi, mystston_state, empty_init, ROT270, "Technos Japan", "Mysterious Stones - Dr. Kick in Adventure (Itisa PCB)", MACHINE_SUPPORTS_SAVE )
