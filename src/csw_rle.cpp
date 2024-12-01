/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  RLE and ZRLE implementation for tape CSW blocks                           |
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
#include <iostream>
#include <stdexcept>
#include <vector>
#include "tipos.h"
#include "log.h"
#include "miniz/miniz.h"
#include "csw_rle.h"


inline u32 u32_min(u32 a, u32 b) {
	return (a < b) ? a : b;
}


ZRLEOutputStream:: ZRLEOutputStream(std::ostream& f) : closed(false), fichero(f), iniciado(false) {
	// Inicializar el flujo de compresión
	stream = {};
	if (mz_deflateInit(&stream, MZ_BEST_COMPRESSION) != MZ_OK) {
		throw std::runtime_error("Error al inicializar el flujo de compresión.");
	}
}


ZRLEOutputStream:: ~ZRLEOutputStream() {
	if (!closed) {
		try {
			close(); // Asegurar cierre limpio
		} catch (...) {
			debug_z("Z:: error al cerrar\n");
		}
	}
	mz_deflateEnd(&stream);
}


void ZRLEOutputStream:: compressBuffer() {
	debug_z("Z:: compress()\n");
	//if (buffer.empty()) return;
	if (buffer.isEmpty()) return;

	//stream.next_in = buffer.data();
	stream.next_in = buffer.bytes;
	//stream.avail_in = static_cast<unsigned int>(buffer.size());
	stream.avail_in = buffer.size();

	// Comprimir hasta que todos los datos sean procesados
	while (stream.avail_in > 0) {
		unsigned char out[Z_BUFFER_SIZE]; // Buffer temporal para los datos comprimidos
		stream.next_out = out;
		stream.avail_out = sizeof(out);

		int status = mz_deflate(&stream, MZ_NO_FLUSH);
		if (status != MZ_OK) {
			debug_z("Z:: error al comprimir\n");
			throw std::runtime_error("Error al comprimir datos con miniz.");
		}

		//compressed.insert(compressed.end(), out, out + sizeof(out) - stream.avail_out);
		fichero.write(reinterpret_cast<char*>(out), sizeof(out) - stream.avail_out);
	}
	//buffer.clear(); // Limpiar el buffer después de comprimir
	buffer.clear();
}


void ZRLEOutputStream:: close() {
	debug_z("Z:: close() pulsesCount=%d\n", pulsesCount);

	if (closed) return;

	// Comprimir los datos restantes en el buffer
	compressBuffer();

	// Finalizar la compresión
	unsigned char out[Z_BUFFER_SIZE];
	int status;

	do {
		//debug_z("Z:: close() avail_out=%d\n", stream.avail_out);
		stream.next_out = out;
		stream.avail_out = sizeof(out);

		status = mz_deflate(&stream, MZ_FINISH);
		if (status < 0) {
			debug_z("Z:: Error al final la compresion\n");
			throw std::runtime_error("Error al finalizar la compresión con miniz.");
		}
		//compressed.insert(compressed.end(), out, out + sizeof(out) - stream.avail_out);
		fichero.write(reinterpret_cast<char*>(out), sizeof(out) - stream.avail_out);
	} while (status != MZ_STREAM_END);

	closed = true;
}


void ZRLEOutputStream:: addByte(BYTE byte) {
	if (closed) {
		debug_z("Z:: no se pueden añadir bytes a un flujo cerrado");
		throw std::runtime_error("No se pueden añadir bytes a un flujo cerrado.");
	}
	buffer.addByte(byte);
	if (buffer.isLimiteAlcanzado()) compressBuffer();

//	buffer.push_back(byte);
//
//	// Comprimir si el buffer alcanza un tamaño significativo
//	if (buffer.size() >= Z_BUFFER_SIZE) {
//		compressBuffer();
//	}
}


void ZRLEOutputStream:: addNumber(u32 n) {
	//debug("+%ld", n);
	if (n > 0x0FF) {
		addByte(0);
		addByte( n & 0xFF );
		addByte( (n & 0xFF00) >> 8 );
		addByte( (n & 0xFF0000) >> 16 );
		addByte( (n & 0xFF000000) >> 24 );
	}
	else {
		BYTE b = n & 0xFF;
		addByte(b);
	}
}


u32 ZRLEOutputStream:: getBytesOutput() {
	return stream.total_out;
}



void ZRLEOutputStream:: changeValue() {
	if (iniciado) {
		//debug("Z:: change_val %d\n", consecutiveLevelCount);
		addNumber(consecutiveLevelCount);
		pulsesCount++;
	}
	else iniciado = true;
}



void ZRLEOutputStream:: addPulseLevel(bool b, u8 ciclos) {
	if (b != lastLevel) {
		changeValue();
		lastLevel = b;
		consecutiveLevelCount = ciclos;
		pulsesCount++;
		//debug_cdt("RLE:: pulsos=%ld  ciclos=%ld\n", pulsesCount, cyclesCount);
	}
	else  consecutiveLevelCount += ciclos;

	cyclesCount += ciclos;
}


u32 ZRLEOutputStream:: getPulses() {
	return pulsesCount;
}


//-------------------------------------------------------------------------------------------


ZRLEInputStream:: ZRLEInputStream(std::istream& f, u32 compressedSize) :
		fichero(f), bytesLeft(compressedSize), output_pos(0), inflateFinished(false)
{
	//if (!fichero->is_open()) {
	//	debug("Error: el archivo no está abierto.");
	//	throw std::runtime_error("Error: el archivo no está abierto.");
	//}
	debug_z("Z:: zrle_input.  compressedSize=%d\n", compressedSize);

	stream = {};
	if (mz_inflateInit(&stream) != MZ_OK) {
		debug_z("Z:: Error al inicializar el flujo de descompresión.");
		throw std::runtime_error("Error al inicializar el flujo de descompresión.");		
	}
}


ZRLEInputStream:: ~ZRLEInputStream() {
	debug_z("Z:: ZRLEInputStream ~");
	mz_inflateEnd(&stream);
}



void ZRLEInputStream:: fillInputBuffer() {
	debug_z("Z:: fill input buffer\n");
	// Leer más datos comprimidos del archivo
	u16 bufferSize = u32_min(Z_BUFFER_SIZE, bytesLeft);

	input_buffer.resize(bufferSize);
	
	fichero.read(reinterpret_cast<char*>(input_buffer.data()), input_buffer.size());
	std::streamsize bytes_read = fichero.gcount();
	bytesLeft -= bytes_read;
	input_buffer.resize(static_cast<size_t>(bytes_read));
	debug_z("Z:: bytesLeft=%d\n", bytesLeft);

	if (bytes_read == 0 && fichero.eof()) {
		throw std::runtime_error("Error: no hay más datos en el archivo para descomprimir.");
	}
	debug_z("Z:: fill input buffer ok\n");
}


void ZRLEInputStream:: decompressChunk() {
	debug_z("Z:: decompress chunk\n");
	if (inflateFinished) {
		throw std::runtime_error("No se puede descomprimir: ya se alcanzó el final del archivo.");
	}

	if (input_buffer.empty()) {
		fillInputBuffer();
	}

	stream.next_in = input_buffer.data();
	stream.avail_in = static_cast<unsigned int>(input_buffer.size());

	BYTE out[Z_BUFFER_SIZE];
	do {
		stream.next_out = out;
		stream.avail_out = sizeof(out);

		int status = mz_inflate(&stream, MZ_NO_FLUSH);
		if (status == MZ_STREAM_END) {
			inflateFinished = true;
		}
		else if (status != MZ_OK) {
			debug_z("Z:: Error durante la descompresión con miniz.");
			throw std::runtime_error("Error durante la descompresión con miniz.");
		}

		size_t bytes_written = sizeof(out) - stream.avail_out;
		output_buffer.insert(output_buffer.end(), out, out + bytes_written);

	} while (stream.avail_out == 0);

	// Eliminar datos procesados del buffer de entrada
	size_t bytes_consumed = input_buffer.size() - stream.avail_in;
	input_buffer.erase(input_buffer.begin(), input_buffer.begin() + bytes_consumed);

	debug_z("Z:: decompress chunk fin %d  \n", stream.total_in);
}


BYTE ZRLEInputStream:: getByte() {
	// Si el buffer descomprimido está vacío, descomprimir un nuevo chunk
	if (output_pos >= output_buffer.size()) {
		output_buffer.clear();
		output_pos = 0;

		if (!inflateFinished) { //} && !inputFinished) {
			try {
				decompressChunk();
			}
			catch (...) {
				debug_z("Z:: no hay mas bloques para descomprimir (1)\n");
				return false;
			}
		} 
		else {
			//throw std::runtime_error("No hay más datos disponibles.");
			debug_z("Z:: no hay mas bloques para descomprimir (2)\n");
			return false;
		}
	}
	// Devolver el siguiente byte descomprimido
	return output_buffer[output_pos++];
}



bool ZRLEInputStream:: hasMore() {
	//debug_z("<%d %d>", output_pos, output_buffer.size());
	if (!inflateFinished) return true;
	//if (output_pos < output_buffer.size()) return true;
	return output_pos < output_buffer.size();
}

u32 ZRLEInputStream:: getPulseCycles() {
	BYTE b0 = getByte();

	if (b0 == 0) {
		b0 = getByte();
		BYTE b1 = getByte();
		BYTE b2 = getByte();
		BYTE b3 = getByte();

		return  (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
	}
	else {
		return b0;
	}
}


//---------------------------------------------------------------------------


RLEInputStream:: RLEInputStream(std::istream& f, u32 compressedSize) : 
	fichero(f), bytesLeft(compressedSize) {}

RLEInputStream:: ~RLEInputStream() {}


BYTE RLEInputStream:: getByte() {
	BYTE b;
	fichero.read(reinterpret_cast<char*>(&b), 1);
	return b;
}


bool RLEInputStream:: hasMore() {
	return bytesLeft > 0;
}

u32 RLEInputStream:: getPulseCycles() {
	BYTE b0 = getByte();

	if (b0 == 0) {
		b0 = getByte();
		BYTE b1 = getByte();
		BYTE b2 = getByte();
		BYTE b3 = getByte();

		return  (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
	}
	else {
		return b0;
	}
};
