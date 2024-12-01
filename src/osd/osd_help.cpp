/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Help on screen                                                            |
|                                                                            |
|  Copyright (c) 2024 Gallegux (gallegux@gmail.com)                          |
|                                                                            |
|  This program is free software: you can redistribute it and/or modify      |
|  it under the terms of the GNU General Public License as published by      |
|  the Free Software Foundation, either version 3 of the License, or         |
|  any later version.                                                        |
|                                                                            |
|  This program is distributed in the hope that it will be useful, but       |
|  WITHOUT ANY WARRANTY; without even the implied warranty of                |
|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              |
|  GNU General Public License for more details.                              |
|                                                                            |
|  You should have received a copy of the GNU General Public License         |
|  along with this program; if not, see <http://www.gnu.org/licenses/>.      |
|                                                                            |
|  If you use this code, please attribute the original source by mentioning  |
|  the author and providing a link to the original repository.               |
|___________________________________________________________________________*/


#include <SDL2/SDL_image.h>
#include <string>
#include "osd.h"
#include "../tipos.h"
#include "../target.h"
#include "../monitor.h"


struct HelpLine {
	std::string text;
	i8 separation = 2;
};


constexpr TColor COLOR_HELP_BG = {0,0,0};
constexpr TColor COLOR_HELP_FG = {250,250,250};


#ifdef TARGET_PC
constexpr u8 HELP_LINES = 20;
const HelpLine ayuda[] = {
	{"Key     Function               Function with AltGr", 0},
	{"-------------------------------------------------------", 3},
	{"F1      Show help              Show credits"},
	{"F2      Insert/Eject disk A    Flip disk A protection"},
	{"F3      Insert/Eject disk B    Flip disk B protection"},
	{"F4      Insert pokes           Exit emulator"},
	{"F5      Insert/Eject cassette  Flip cassette protection"},
	{"F6      Play tape              Play & Record tape"},
	{"F7      Flip pause cassette    Stop tape"},
	{"F8      Load snapshot          Save snapshot"},
	{"F9      Load CPC settings      Save current CPC settings"},
	{"F10     Save screen            Flip scanlines"},
	{"F11     Change palette         Change palette variant"},
	{"F12     Reset CPC              Select CPC/ram/roms"},
	//{"BlqDs   Show last printed job   Show printed jobs"},
	{"PAUSE   Play/Pause emulation   Normal/Turbo speed"},
	{"Begin   Rewind cassette        Rewind cassette one track"},
	{"End     Wind cassette          Wind cassette one track"},
	{"AvPag   Volume up              Normal/Faster loads"},
	{"RePag   Volume down            Change Joystick keys",16},
	{"                  Press ESC to quit"}
};
#endif

#ifdef TARGET_LILYGO
constexpr u8 HELP_LINES = 19;
const HelpLine ayuda[] = {
	{"Key     Function                Function with AltGr", 0},
	{"--------------------------------------------------------", 3},
	{"F1      Show help               Show fast help"},
	{"F2      Insert/Eject disk A     Flip disk A protection"},
	{"F3      Insert/Eject disk B     Flip disk B protection"},
	{"F4      Insert/Eject cassete    Flip cassette protection"},
	{"F5      Play tape               Play & Record tape"},
	{"F6      Flip pause tape         Stop tape"},
	{"F7      Load snapshot           Save snapshot"},
	{"F8      Pokes menu"},
	{"F9      Select CPC/ram/roms     Save screen"},
//	{"F10     "},
	{"F11     Change colour balance   Flip scanlines"},
	{"F12     Reset CPC               Reset ESP32"},
//	{"Impr    Show last printed job   Show printed jobs"},
//	{"BlqDs   "},
	{"PAUSE   Play/Pause emulation    Normal/Turbo speed,"},
	{"Begin   Rewind cassette         Rewind cassette one trak"},
	{"End     Wind cassette           Wind cassette one track"},
	{"AvPag   Volume up"},
	{"RePag   Volume down", 16},
	{"                  Press ESC to quit"}
};
#endif



void OSD:: showHelp() {
//#define MON_W MONITOR_WIDTH/2
#define MAX_CHARS_LINE (MONITOR_WIDTH / (FONT_WIDTH*2))
	u8 maxLen = 0;
	u16 height = 0;
	for (u8 a = 0; a < HELP_LINES; a++) {
		if (ayuda[a].text.size() > maxLen) maxLen = ayuda[a].text.size();
		height += (ayuda[a].separation + FONT_HEIGHT);
	}

	// texto centrado
	u16 iniX = (MONITOR_WIDTH - maxLen*FONT_WIDTH*2) / 2; 
	u16 iniY = (MONITOR_HEIGHT - height*2) / 2;

	u16 rectX = iniX - FONT_WIDTH*4;
	u16 rectY = iniY - FONT_HEIGHT*2;
	u16 rectW = maxLen*FONT_WIDTH*2 + 2*4*FONT_WIDTH;
	u16 rectH = height*2 + 2*2*FONT_HEIGHT;

	setColors(COLOR_HELP_BG, COLOR_HELP_FG);
	drawFilledRectangle(rectX, rectY, rectW, rectH);

	//iniX = FONT_WIDTH*2 * ((MAX_CHARS_LINE - maxLen) / 2);
	u16 linea = iniY+2 ;// + FONT_WIDTH*2;
	for (u8 a = 0; a < HELP_LINES; a++) {
		writeFastText(iniX, linea, ayuda[a].text);
		linea += (FONT_HEIGHT + ayuda[a].separation) << 1;
	}
#undef MON_W

	updateUI();
}

