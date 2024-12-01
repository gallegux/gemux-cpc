/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Copyright on OSD                                                          |
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


#include <string>
#include <string_view>
#include "osd.h"
#include "../tipos.h"
#include "../target.h"



constexpr TColor CR_WINDOW_BG = {250,250,250};
constexpr TColor CR_TITLE_GEMUX1_FG = {222,222,222};
constexpr TColor CR_VERSION_FG = {128,128,128};
constexpr TColor CR_BY_FG = {0,128,192};
constexpr TColor CR_BY2_FG = {0,192,128};

constexpr TColor LOGO_BACKGROUND_COLOR = {16,16,16};
constexpr TColor LOGO_TEXT_COLOR = {240,240,240};
constexpr TColor LOGO_BAR1_COLOR = {220,0,0};
constexpr TColor LOGO_BAR2_COLOR = {0,190,0};
constexpr TColor LOGO_BAR3_COLOR = {0,25,255};


void bold2(OSD* osd, u8 row, const std::string_view& text, TColor color, i8 dx, i8 dy) {
	for (u8 i = 0; i < 4; i++) {
		osd->writeFastTextCentered(row, text, color, dx, dy);
		dx += 2;
	}
}

void bold(OSD* osd, u8 row, const std::string_view& text, TColor color) {
	bold2(osd, row, text, color, 4, -4);
	bold2(osd, row, text, color,-6,  6);
}


void OSD:: showCredits() {
	u8 x = 7;  // primera linea

	setBackground(CR_WINDOW_BG);

#ifdef TARGET_PC
	//drawFilledRectangleC(15, r, 35, 20);
	drawFilledRectangleC(15, x, 34, 16);
#endif

#ifdef TARGET_LILYGO
	drawFilledRectangleC(15, 7, 34, 20);
#endif

	setBackground(LOGO_BACKGROUND_COLOR);
	drawFilledRectangleC(24, x+2, 16, 3);

	writeFastTextCentered(x + 3, "GEMUX CPC ///", LOGO_TEXT_COLOR);
	bold(this, x+3, "          /  ", LOGO_BAR1_COLOR);
	bold(this, x+3, "           / ", LOGO_BAR2_COLOR);
	bold(this, x+3, "            /", LOGO_BAR3_COLOR);
	writeFastTextCentered(x + 7, "version 0.2-a", CR_VERSION_FG);
	writeFastTextCentered(x + 10, "(c) 2024 Gallegux", CR_BY_FG);
	writeFastTextCentered(x + 13, "UI inspired by ESPectrum", CR_BY2_FG);

	updateUI();
}