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


#include <fstream>
#include <filesystem>
#include <string>
#include <algorithm>
#include "tipos.h"
#include "util.h"
#include "file_util.h"
#include "log.h"



//inline void file_consumeBytes(std::fstream& fichero, u32 n) {
//	fichero.seekg(n, std::ios::cur);
//}

//inline void file_nextByte(std::fstream& fichero, BYTE* b) {
//	fichero.read(reinterpret_cast<char*>(b), 1);
//}

//inline void file_nextWord(std::fstream& fichero, u16* w) {
//	fichero.read(reinterpret_cast<char*>(w), 2);
//}

void file_next3Bytes(std::fstream& fichero, u32* v) {
	BYTE b;
	fichero.read(reinterpret_cast<char*>(&b), 1);	*v = b;
	fichero.read(reinterpret_cast<char*>(&b), 1);	*v |= b << 8;
	fichero.read(reinterpret_cast<char*>(&b), 1);	*v |= b << 16;
}

//inline void file_nextDWord(std::fstream& fichero, u32* v) {
//	fichero.read(reinterpret_cast<char*>(v), 4);
//}

BYTE file_nextByte(std::fstream& fichero) {
	BYTE b;
	file_nextByte(fichero, &b);
	return b;
}

u16 file_nextWord(std::fstream& fichero) {
	u16 v;
	file_nextWord(fichero, &v);
	return v;
}

u32 file_next3Bytes(std::fstream& fichero) {
	u32 v;
	file_next3Bytes(fichero, &v);
	return v;
}

u32 file_nextDWord(std::fstream& fichero) {
	u32 v;
	file_nextDWord(fichero, &v);
	return v;
}



bool file_exists(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
	//printf("-------%d %d\n", file.good(), file.is_open());
    return file.good(); // Devuelve true si el archivo se pudo abrir
}



void file_copyPiece(std::fstream& in, u32 bytes, std::fstream& out) {
	constexpr u32 COPY_BUFFER_SIZE = 8192;
	char buffer[COPY_BUFFER_SIZE];

	u32 bytesBuffer;

	while (bytes > 0) {
		bytesBuffer = min_u32(COPY_BUFFER_SIZE, bytes);
		bytes -= bytesBuffer;
		in.read(buffer, bytesBuffer);
		out.write(buffer, bytesBuffer);
	}
}


std::vector<std::string> file_getFiles(std::string& directorio, std::string& extension) {
    std::vector<std::string> files;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(directorio)) {
            if (entry.is_regular_file() && entry.path().extension() == extension) {
                files.push_back(entry.path().filename().string());
            }
        }
        std::sort(files.begin(), files.end());
    } 
    catch (const std::filesystem::filesystem_error& e) {
		debug("Error: %s\n", e.what());
        //std::cerr << "Error: " << e.what() << std::endl;
    }

    return files;
}


std::string file_getDirectory(const std::string& rutaArchivo) {
    std::filesystem::path ruta(rutaArchivo);
    return ruta.parent_path().string();  // Obtener el directorio padre como string
}


bool file_checkSize(std::ifstream& f, const u64 size) {
	f.seekg(0, std::ios::end);
	std::streampos _size = f.tellg();
	long long __size = _size;
	f.seekg(0, std::ios::beg);

	return _size == size;
}

