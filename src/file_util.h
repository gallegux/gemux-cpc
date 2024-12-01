/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Helper file functions                                                     |
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


#include <fstream>
#include "tipos.h"


inline void file_consumeBytes(std::fstream& fichero, u32 n) {
	fichero.seekg(n, std::ios::cur);
}

inline void file_nextByte(std::fstream& fichero, BYTE* b) {
	fichero.read(reinterpret_cast<char*>(b), 1);
}

inline void file_nextWord(std::fstream& fichero, u16* w) {
	fichero.read(reinterpret_cast<char*>(w), 2);
}

void file_next3Bytes(std::fstream& fichero, u32* v);

inline void file_nextDWord(std::fstream& fichero, u32* v) {
	fichero.read(reinterpret_cast<char*>(v), 4);
}

BYTE file_nextByte(std::fstream& fichero);

u16 file_nextWord(std::fstream& fichero);

u32 file_next3Bytes(std::fstream& fichero);

u32 file_nextDWord(std::fstream& fichero);


bool file_exists(const std::string& filename);

void file_copyPiece(std::fstream& in, u32 bytes, std::fstream& out);

void file_getFiles(std::vector<std::string>& lista, std::string& directorio, std::string& extension);

std::string file_getDirectory(const std::string& rutaArchivo);

bool file_checkSize(std::ifstream& f, const u64 size);
