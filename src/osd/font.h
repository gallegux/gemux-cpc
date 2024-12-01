/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Font for OSD                                                              |
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


#pragma once


#include "../tipos.h"


constexpr u8 NUM_CHARS   = 96;

constexpr u8 FONT_WIDTH  = 6;
constexpr u8 FONT_HEIGHT = 9;

constexpr u8 NUM_PIXELS_CHAR = FONT_WIDTH * FONT_HEIGHT;

constexpr char NOT_PRINTABLE_CHAR = 127;
constexpr char ESCAPE_CHAR = -61;



const bool* font_getPtrChar(u8 c);

