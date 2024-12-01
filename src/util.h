/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Helper functions                                                          |
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


#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include "tipos.h"


inline u16 min_u16(u16 a, u16 b) {
	return (a < b) ? a : b;
}

inline u32 min_u32(u32 a, u32 b) {
	return (a < b) ? a : b;
}


void toUpperCase(std::string& str);

void toLowerCase(std::string& str);

// Sobrecarga para comparar con std::string prefix
bool startsWith(const std::string& str, const std::string& prefix);

// Sobrecarga para comparar con const char* prefix
bool startsWith(const std::string& str, const char* prefix);

u32 random(u32 max);

bool toNumber(const std::string& str, int* valor);

std::string strDate();

uint32_t colorValue(TColor c);

void concat(std::string& dest, std::string_view s1, std::string s2);

void concat(std::string& dest, std::string_view s1, std::string s2, std::string s3);

std::string concat(std::string_view s1, std::string s2);



void centerText(std::string& texto, u8 ancho);


std::string toHex(u32 number, u8 width);


std::string trim(const std::string& str);

void trim_string(std::string& str);

bool endsWith(const std::string& str, const std::string& suffix);
