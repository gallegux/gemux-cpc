/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Screen display implementation                                             |
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

#include <string>
#include <SDL2/SDL.h>
#include "tipos.h"
#include "osd/osd.h"
#include "osd/font.h"


#define LINEAS_VISIBLES		(5+25+4)
#define COLUMNAS_VISIBLES	(4+40+4)

#define MAX_RASTER_X		(COLUMNAS_VISIBLES*8*2)
#define MIN_RASTER_Y		(3*8)		// 3 lineas superiores no se visualizan
#define MAX_RASTER_Y		(MIN_RASTER_Y + LINEAS_VISIBLES*8)

constexpr u16 MONITOR_WIDTH  = MAX_RASTER_X;
constexpr u16 MONITOR_HEIGHT = (MAX_RASTER_Y-MIN_RASTER_Y)*2;

constexpr u8 MARGEN_X = 8;
constexpr u8 MARGEN_Y = 16;

constexpr u8 LINEAS_ACTUALIZACION = 52;

constexpr u8 NUM_PALETAS_MONITOR = 14;
constexpr u8 COLOR_PALETTE = 0;
constexpr u8 GREEN_PALETTE = 1;
constexpr u8 GRAY_PALETTE  = 2;
constexpr u8 ORANGE_PALETE = 3;

constexpr u8 OSD_COLS = MONITOR_WIDTH /2 /FONT_WIDTH;
constexpr u8 OSD_ROWS = MONITOR_HEIGHT /2 /FONT_HEIGHT;


class OSD;



const TColor paleta_color_teo[32] = {
	{127, 127, 127},   // fw.color=13
	{127, 127, 127},   // fw.color=
	{0, 255, 127},   // fw.color=19
	{255, 255, 127},   // fw.color=25
	{0, 0, 127},   // fw.color=1
	{255, 0, 127},   // fw.color=7
	{0, 127, 127},   // fw.color=10
	{255, 127, 127},   // fw.color=16
	{255, 0, 127},   // fw.color=
	{255, 255, 127},   // fw.color=
	{255, 255, 0},   // fw.color=24
	{255, 255, 255},   // fw.color=26
	{255, 0, 0},   // fw.color=6
	{255, 0, 255},   // fw.color=8
	{255, 127, 0},   // fw.color=15
	{255, 127, 255},   // fw.color=17
	{0, 0, 127},   // fw.color=
	{0, 255, 127},   // fw.color=
	{0, 255, 0},   // fw.color=18
	{0, 255, 255},   // fw.color=20
	{0, 0, 0},   // fw.color=0
	{0, 0, 255},   // fw.color=2
	{0, 127, 0},   // fw.color=9
	{0, 127, 255},   // fw.color=11
	{127, 0, 127},   // fw.color=4
	{127, 255, 127},   // fw.color=22
	{127, 255, 0},   // fw.color=21
	{127, 255, 255},   // fw.color=23
	{127, 0, 0},   // fw.color=3
	{127, 0, 255},   // fw.color=5
	{127, 127, 0},   // fw.color=12
	{127, 127, 255}   // fw.color=14
};


// https://grimware.org/doku.php/documentations/devices/gatearray
const TColor paleta_color_real[32] = {
	{ 110, 125, 106 },
	{ 110, 122, 109 },
	{ 000, 242, 106 },
	{ 242, 242, 109 },
	{ 000, 002, 106 },
	{ 240, 002, 104 },
	{ 000, 120, 104 },
	{ 242, 125, 106 },
	{ 242, 002, 104 },
	{ 242, 242, 106 },
	{ 242, 242, 013 },
	{ 255, 242, 248 },
	{ 242, 004, 006 },
	{ 242, 002, 243 },
	{ 242, 125, 013 },
	{ 250, 001, 248 },
	{ 000, 002, 104 },
	{ 002, 242, 106 },
	{ 002, 240, 001 },
	{ 014, 242, 241 },
	{ 000, 002, 001 },
	{ 012, 002, 243 },
	{ 002, 120, 001 },
	{ 012, 122, 243 },
	{ 105, 002, 104 },
	{ 112, 242, 106 },
	{ 112, 245, 003 },
	{ 112, 242, 243 },
	{ 107, 002, 001 },
	{ 107, 002, 241 },
	{ 110, 122, 001 },
	{ 110, 122, 246 }
};


const u8 niveles_monocromo_1[32] = {
	143, 143, 191, 240, 47, 96, 120, 167, 96, 240, 231, 247, 88, 103, 160, 176, 47, 191, 184, 199, 40, 55, 111, 128, 72, 216, 208, 223, 64, 79, 135, 152
};

const u8 niveles_monocromo_2[32] = {
	146, 146,192,247,45,101,118,174,101,247,232,255,86,116,159,189,45,192,177,206,30,60,104,133,73,219,205,234,58,88,131,161
};




class Monitor
{
	OSD* osd;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	uint32_t* pixels; // = new uint32_t[MONITOR_WIDTH * MONITOR_HEIGHT];

	i32 raster_x;
	i32 raster_y;
	i16 vdur_min;
	u16 contadorActualizacion;

	bool scanlines = true;
	u8 numPaleta;
	u8 variantePaleta = 3;
	//const TColor* paleta;
	TColor paleta[32];

	TColor osd_color; // color actual de los mensajes osd

public:

	Monitor();

	//void setRenderer(SDL_Renderer* renderer);
	//void setTexture(SDL_Texture* texture);
	//void setWindow(SDL_Window* window);
	void setSDLObjects(SDL_Renderer* renderer, SDL_Texture* texture, uint32_t* pixels);
	void setOSD(OSD* osd);

	void setPalette(u8 numPaleta);
	void setPaletteVariant(u8 numVariant);
	void setPalette(u8 numPaleta, u8 varPaleta);
	void updatePalette();
	void changePalette();
	void changePaletteVariant();

	std::string getPaletteName();
	u8   getPalette();
	u8   getPaletteVariant();

	void useScanlines(bool u);
	void flipScanlines();
	bool getScanlines();

	void update(u8* arrayTintas);  // la mascara es para que no sobrepase el rango de 16k o 32k
	void update(bool hsync, bool vsync);
	void update(u8 colorHwBorde);

	void nextLine();	// lleva el raster al principio de la linea
	void nextFrame(i16 vdur_min);	// lleva el raster a la 1a fila

	void printPixel(u8 color);

	bool saveScreen(const std::string& fichero);  // return true si se ha grabado

	//void printHelp();
	//void printText(u16 x, u16 y, const std::string& texto);

};



u32 getArrayIndex(u16 x, u16 y);
