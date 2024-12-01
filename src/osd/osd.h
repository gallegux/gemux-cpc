/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  OSD messages manager and file selection                                   |
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
#include <vector>
#include <filesystem>
#include <SDL2/SDL.h>
#include "font.h"
#include "../tipos.h"
#include "../compilation_options.h"


constexpr u32 MAX_CICLOS_MENSAJE_OSD = 2*50*20000; // nº de nops

constexpr u32 POPOP_MESSAGE_TIME_MS = 50 * 20; // nº de nops en 1 milisegundo
constexpr u32 POPOP_MESSAGE_DEFAULT_TIME_MS = 2000; // 2 segundos
constexpr TColor POPUP_MESSAGE_COLOR = {250,250,250};


class OSD_Window; // declarada en osd_window.h


typedef struct {
	std::string message;
	u8 len = 0; // longitud del mensaje
	u8 character; // caracter actual
	u8 chrCol;	// columna del caracter
	u8 chrRow;  // fila del caracter
	u16 pixelsLen;  // anchura del mensaje en pixeles
	u32 screenPixel;
	const bool* chrPixel;
	i32 displayCounter = 0;
	TColor color = {255,255,255};

	void set(const std::string& m);
} OSD_PopupMessage;


typedef struct {
	u32 numPixel;
	u16 width, height;
	uint32_t* buffer;
} OSD_WindowStack;



class OSD
{
	uint32_t backgroundColor = 0xFFFFFF;
	uint32_t foregroundColor = 0xddddFF;

	SDL_Renderer* renderer;
	SDL_Texture* texture;
	uint32_t* pixels;

	OSD_PopupMessage* popupMessage;
	std::vector<OSD_WindowStack> windowStack;

	void _backStack();

public:
	void setBackground(TColor c);
	void setForeground(TColor c);
	void setColors(TColor background, TColor foreground);

	void drawWindowFrameC(u8 col, u8 row, u8 width, u8 height, TColor borderColor);
	void drawWindowFrameC(u8 col, u8 row, u8 width, u8 height, TColor borderColor, TColor shadowColor);
	void drawFilledRectangle(u16 x, u16 y, u16 width, u16 height); // al pixel
	void drawFilledRectangleC(u8 col, u8 row, u8 width, u8 height); // al caracter

	std::string centerText(const std::string& texto, u8 ancho);

	void writeFastText(u16 x, u16 y, const std::string& text); // no pinta el color de fondo
	void writeFastTextC(u8 col, u8 row, const std::string& text); // no pinta el color de fondo
	void writeFastTextC(u8 col, u8 row, const std::string_view& text, TColor color);
	void writeFastTextC(u8 col, u8 row, const std::string_view& text, TColor color, i8 dx, i8 dy);
	void writeFastTextCentered(u8 row, const std::string_view& text, TColor color);
	void writeFastTextCentered(u8 row, const std::string_view& text, TColor color, i8 dx, i8 dy);
	
	void writeText(u16 x, u16 y, const std::string& text);
	void writeTextC(u8 col, u8 row, const std::string& text);

	void printCursor(); // pinta el cursor y la letra en video inverso

	void updateUI(); // actualiza la pantalla con lo dibujado


public:
	OSD(SDL_Renderer* osd_renderer, SDL_Texture* osd_texture, uint32_t* osd_pixels);

	void update(u8 nops);	// para los mensajes emergentes

	void setPopupMessage(std::string msg, u32 timeMS);
	void setPopupMessage(std::string msg);
	void setPopupColor(TColor c);

	void printPopupMessage(u16 raster_x, u16 raster_y, u32 numPixel); 
	// metodo especialito, pinta los pixeles necesarios

	void showHelp();
	void showCredits();
	void waitESC(); // espera a que se pulse ESC

	void push(OSD_Window& w);
	void pop();
	void backStack();
	
};



u16 getXchar(u8 col);

// obtiene la coordenada Y para la fila de caracteres
u16 getYchar(u8 row);

u16 getXforCenter(u8 textLength);
