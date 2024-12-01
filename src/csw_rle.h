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


#pragma once


#include <vector>
#include <stdexcept>
#include <iostream>
#include "miniz/miniz.h"
#include "tipos.h"

constexpr u8 Z_BUFFER_MARGIN = 5;
constexpr u16 Z_BUFFER_SIZE = 0xFFFF - Z_BUFFER_MARGIN;


typedef struct {
	BYTE bytes[Z_BUFFER_SIZE+Z_BUFFER_MARGIN]; // 5=margen
	u16 cont = 0;

	inline bool isLimiteAlcanzado() {
		return cont >= (Z_BUFFER_SIZE + Z_BUFFER_MARGIN);
	}
	inline void clear() {
		cont = 0;
	}
	inline u16 size() {
		return cont;
	}
	inline bool isEmpty() {
		return cont == 0;
	}
	inline void addByte(BYTE b) {
		bytes[cont++] = b;
	}
} T_ZBufferOut;



class I_RLEOutputStream
{
public:
	virtual void addPulseLevel(bool pulseLevel, u8 cycles) = 0;
	virtual u32 getBytesOutput() = 0; // bytes de salida
	virtual u32 getPulses() = 0; // pulsos
    virtual void close() = 0;
};


class I_RLEInputStream
{
public:
	virtual bool hasMore() = 0;  // indica si se han obtenido todos los datos descomprimidos
    virtual u32 getPulseCycles() = 0;  // ciclos del pulso
};



class ZRLEOutputStream : public I_RLEOutputStream
{

private:
	std::ostream& fichero;
    //std::vector<unsigned char> buffer;        // Buffer interno para los datos no comprimidos
	T_ZBufferOut buffer;
    mz_stream stream;                         // Estructura de compresión de miniz
    bool closed;                           // Indicador de cierre

	bool lastLevel;
	bool iniciado;
	u32 consecutiveLevelCount, pulsesCount, cyclesCount, size; // bytes

    void compressBuffer();
    void addByte(unsigned char byte);

	void changeValue();

public:
    ZRLEOutputStream(std::ostream& f);

    ~ZRLEOutputStream();

	void addNumber(u32 n);
	void addPulseLevel(bool b, u8 ciclos);
	u32 getBytesOutput(); // bytes de salida
	u32 getPulses(); // pulsos
	
    void close();

};



class ZRLEInputStream : public I_RLEInputStream
{
private:
    std::istream& fichero;                 // Referencia al archivo de entrada
    mz_stream stream;                         // Estructura de descompresión de miniz
    std::vector<unsigned char> input_buffer;  // Buffer de datos comprimidos
    std::vector<unsigned char> output_buffer; // Buffer de datos descomprimidos
    size_t output_pos;                        // Posición actual en el buffer descomprimido
    bool inflateFinished;                         // Indicador de si se terminó de descomprimir
    i32 bytesLeft; // indica cuantos bytes de la entrada quedan por descomprimir
	//bool inputFinished; // indica si se han leido todos los bytes (compressedSize)
	//u16 buffer_pos;  // posicion de lectura en el buffer descomprimido

	void fillInputBuffer();
	void decompressChunk();
	BYTE getByte();

public:
	ZRLEInputStream(std::istream& f, u32 compressedSize);
	~ZRLEInputStream();

    u32 getPulseCycles() override;  
	bool hasMore() override;
    // return true si se ha obtenido el dato y no ha llegado al final
    // false si ya no hay datos

};



class RLEInputStream : public I_RLEInputStream
{
	std::istream& fichero;
	i32 bytesLeft; // indica cuantos bytes de la entrada quedan por descomprimir
	bool inputFinished; // indica si se han leido todos los bytes (compressedSize)
	BYTE getByte();

public:
	RLEInputStream(std::istream& f, u32 compressedSize);
	~RLEInputStream();

    u32 getPulseCycles() override;  
	bool hasMore() override;
};
