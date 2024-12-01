/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  OSD messages manager implementation                                       |
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
#include <string>
#include <vector>
#include <cstring> // Para std::memcpy
#include <algorithm> // Para std::fill_n
#include <SDL2/SDL.h>
#include "osd.h"
#include "osd_window.h"
#include "font.h"
#include "util.h"
#include "../tipos.h"
#include "../target.h"
#include "../monitor.h"
#include "../util.h"
#include "../log.h"

/*
solo tenemos caracteres ascii
lo caracteres extendidos (con codigo >= 127) los representamos con el mismo simbolo

en los nombres de ficheros, los caracteres con tildes de distintos tipos, enyes, etc 
se usan secuencias de escape y parece que ese byte es el 195
*/

#ifdef TARGET_PC
extern bool GLOBAL_QUIT;
#endif


#define debugChars(s) { \
	debug_osd("OSD:: [%s]=", s.c_str()); \
	for (u8 i = 0; i < s.size(); i++) debug_osd(" %d", s.at(i)); \
	debug_osd("\n"); \
}




// obtiene la coordenada X para la columna de caracteres
inline u16 getXchar(u8 col) {
	return (col * FONT_WIDTH * 2);
}

// obtiene la coordenada Y para la fila de caracteres
inline u16 getYchar(u8 row) {
	return (row * FONT_HEIGHT * 2);
}


u16 getXforCenter(u8 textLength) {
	// texto centrado
	return (MONITOR_WIDTH - textLength*FONT_WIDTH*2) / 2; 
}


std::string OSD:: centerText(const std::string& texto, u8 ancho) {
    if (texto.size() >= ancho) {
        return texto;  // Si el texto ya es más largo o igual al ancho, no se modifica
    }

    u8 espacios_total = ancho - texto.size();  // Espacio total a distribuir
    u8 espacios_izquierda = espacios_total / 2;  // Mitad del espacio a la izquierda
    u8 espacios_derecha = espacios_total - espacios_izquierda;  // Resto del espacio a la derecha

    return std::string(espacios_izquierda, ' ') + texto + std::string(espacios_derecha, ' ');
}


void OSD_PopupMessage:: set(const std::string& m) {
	message = m;
	len = m.size();
	character = 0;
	chrCol = 0;
	chrRow = 0;
	//pixelsLen = (FONT_WIDTH * len) << 1;  // <<1 --> x2
	pixelsLen = FONT_WIDTH * len;
	displayCounter = MAX_CICLOS_MENSAJE_OSD;
}



OSD:: OSD(SDL_Renderer* renderer, SDL_Texture* texture, uint32_t* pixels) {
	this->renderer = renderer;
	this->texture = texture;
	this->pixels = pixels;

	popupMessage = nullptr;
}


void OSD:: update(u8 nops) {
	//debug_osd("%ld\n", osd_message.displayCounter);
	//if (popupMessage.displayCounter >= 0) {
	if (popupMessage != nullptr) {
		popupMessage->displayCounter -= nops;
		if (popupMessage->displayCounter < 0) {
			delete popupMessage;
			popupMessage = nullptr;
		}
	}
}


void OSD:: setBackground(TColor color) {
	//backgroundColor =  color.r << 16  |  color.g << 8  |  color.b ;
	backgroundColor = colorValue(color);
}

void OSD:: setForeground(TColor color) {
	//foregroundColor =  color.r << 16  |  color.g << 8  |  color.b ;
	foregroundColor = colorValue(color);
}

void OSD:: setColors(TColor bg, TColor fg) {
	setBackground(bg);
	setForeground(fg);
}


void OSD:: updateUI() {
	SDL_UpdateTexture(texture, NULL, pixels, MONITOR_WIDTH*sizeof(uint32_t));
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}


void OSD:: setPopupMessage(std::string msg, u32 timeMS) {
	if (popupMessage == nullptr) popupMessage = new OSD_PopupMessage;
	popupMessage->set(msg);
	popupMessage->displayCounter = timeMS * POPOP_MESSAGE_TIME_MS;
}

void OSD:: setPopupMessage(const std::string msg) {
	setPopupMessage(msg, POPOP_MESSAGE_DEFAULT_TIME_MS);
}



// escritura de texto mas rapida
// solo se pintan los puntos de las letras, no modificando el fondo
void OSD:: writeFastText(u16 x, u16 y, const std::string& text)
{
	uint32_t po = (y) * MONITOR_WIDTH + (x);
	uint32_t p = po;
	u8 textSize = text.size();
	u8 sy, sx;
	char chr;
	const bool* chrPixel;
	//debugChars(text);

	for (u8 c = 0, ci = 0; c < textSize; c++) {
		// ci=contador de caracteres que se imprimen (incluido el espacio)
		// c=contador de caracteres de la cadena
		chr = text[c];
		if (chr == ESCAPE_CHAR) chr = NOT_PRINTABLE_CHAR;
		chrPixel = font_getPtrChar(chr);
		p = po + ci*2*FONT_WIDTH;
		ci++;

		for (sy = 0; sy < FONT_HEIGHT; sy++) {
			for (sx = 0; sx < FONT_WIDTH; sx++) {
				if (*chrPixel)
					pixels[p] = pixels[p + 1] = pixels[p + MONITOR_WIDTH] = pixels[p + MONITOR_WIDTH + 1] = foregroundColor;
				p += 2;
				chrPixel++;
			}
			p += MONITOR_WIDTH*2 - FONT_WIDTH*2;
		}
		if (text[c] == ESCAPE_CHAR) c++;
	}
}

void OSD:: writeText(u16 x, u16 y, const std::string& text)
{
	uint32_t po = (y) * MONITOR_WIDTH + (x);
	uint32_t p = po;
	uint32_t color;
	u8 textSize = text.size();
	u8 sy, sx;
	char chr;
	const bool* chrPixel;
	//debugChars(text);

	for (u8 c = 0, ci = 0; c < textSize; c++) {
		// ci=contador de caracteres que se imprimen (incluido el espacio)
		// c=contador de caracteres de la cadena
		chr = text[c];
		if (chr == ESCAPE_CHAR) chr = NOT_PRINTABLE_CHAR;
		chrPixel = font_getPtrChar(chr);
		p = po + ci*2*FONT_WIDTH;
		ci++;

		for (sy = 0; sy < FONT_HEIGHT; sy++) {
			for (sx = 0; sx < FONT_WIDTH; sx++) {
				color = (*chrPixel) ? foregroundColor : backgroundColor;
				pixels[p] = pixels[p + 1] = pixels[p + MONITOR_WIDTH] = pixels[p + MONITOR_WIDTH + 1] = color;
				p += 2;
				chrPixel++;
			}
			p += MONITOR_WIDTH*2 - FONT_WIDTH*2;
		}		
		if (text[c] == ESCAPE_CHAR) c++;
	}
}


void OSD:: writeFastTextC(u8 col, u8 row, const std::string& text) {
	writeFastText( getXchar(col), getYchar(row), text);
}


void OSD:: writeFastTextC(u8 col, u8 row, const std::string_view& text, TColor color) {
	setForeground(color);
	std::string s {text};
	writeFastTextC(col, row, s);
}


void OSD:: writeFastTextC(u8 col, u8 row, const std::string_view& text, TColor color, i8 dx, i8 dy) {
	setForeground(color);
	std::string s {text};
	writeFastText( getXchar(col) + dx, getYchar(row) + dy, s);
}


void OSD:: writeFastTextCentered(u8 row, const std::string_view& text, TColor color) {
	setForeground(color);
	//u16 x = getXforCenter(text.size());
	//u16 y = FONT_HEIGHT * 2 * row;
	std::string s {text};
	writeFastText(getXforCenter(text.size()), getYchar(row), s);
}


void OSD:: writeFastTextCentered(u8 row, const std::string_view& text, TColor color, i8 dx, i8 dy) {
	setForeground(color);
	//u16 x = getXforCenter(text.size());
	//u16 y = FONT_HEIGHT * 2 * row;
	std::string s {text};
	writeFastText(getXforCenter(text.size()) + dx, getYchar(row) + dy, s);
}




void OSD:: writeTextC(u8 col, u8 row, const std::string& text) {
	writeText( getXchar(col), getYchar(row), text);
}




void OSD:: drawWindowFrameC(u8 col, u8 row, u8 width, u8 height, TColor borderColor) {
	//debug_osd("col,row=%dx%d\n", col, row);
	uint32_t* po;
	uint32_t* po2;
	uint32_t color = colorValue(borderColor);
	u32 x = getXchar(col) - 2;
	u32 y = getYchar(row);
	u32 w = getXchar(width) + 2;
	u32 h = getYchar(height);

	// lineas verticales
	po = &pixels[getArrayIndex(x, y)]; // izq
	po2 = &pixels[getArrayIndex(x + width, y)]; // der
	for (u16 i = 0; i < h; i++, po += MONITOR_WIDTH) {
		*po = *(po+1) = *(po+w) = *(po+w+1) = color;
	}

	y -= 2;
	po = &pixels[getArrayIndex(x,y)];
	w += 2;
	//h += 2;
	// linea superior
	std::fill_n(po, w, color);
	std::fill_n(po + MONITOR_WIDTH, w, color);
	// linea inferior
	h += 2;
	std::fill_n(po + MONITOR_WIDTH*h, w, color);
	std::fill_n(po + MONITOR_WIDTH*(h+1), w, color);
}

void OSD:: drawWindowFrameC(u8 col, u8 row, u8 width, u8 height, TColor borderColor, TColor shadowColor) {
	//debug_osd("col,row=%dx%d\n", col, row);
	uint32_t* po;
	uint32_t* po2;
	uint32_t _borderColor = colorValue(borderColor);
	uint32_t _shadowColor = colorValue(shadowColor);
	u32 x = getXchar(col) - 2;
	u32 y = getYchar(row);
	u32 w = getXchar(width) + 2;
	u32 h = getYchar(height) + 2;

	// lineas verticales
	po = &pixels[getArrayIndex(x, y)]; // izq
	po2 = &pixels[getArrayIndex(x + width, y)]; // der
	for (u16 i = 0; i < h; i++, po += MONITOR_WIDTH, po2 += MONITOR_WIDTH) {
		*po = *(po+1) = *(po+w) = *(po+w+1) = _borderColor;
		*(po+w+2) = *(po+w+3) = _shadowColor; // sombra
	}

	y -= 2;
	po = &pixels[getArrayIndex(x,y)];
	w += 2;
	//h += 2;
	// linea superior
	std::fill_n(po, w, _borderColor);
	std::fill_n(po + MONITOR_WIDTH, w, _borderColor);
	// linea inferior
	std::fill_n(po + MONITOR_WIDTH*h, w, _borderColor);
	std::fill_n(po + MONITOR_WIDTH*(h+1), w, _borderColor);
	// sombra
	std::fill_n(po + MONITOR_WIDTH*(h+2)+2, w, _shadowColor);
	std::fill_n(po + MONITOR_WIDTH*(h+3)+2, w, _shadowColor);
}



void OSD:: drawFilledRectangle(u16 px, u16 py, u16 width, u16 height) {
	uint32_t* po = &pixels[getArrayIndex(px,py)];

	for (u16 y = 0; y < height; y++, po += MONITOR_WIDTH) {
		std::fill_n(po, width, backgroundColor);
	}
}



// rectangulos alineados al caracter
void OSD:: drawFilledRectangleC(u8 col, u8 row, u8 width, u8 height) {
	drawFilledRectangle(getXchar(col), getYchar(row), getXchar(width), getYchar(height));
}


//--------------------------------------------------------------------------------------

void OSD:: printPopupMessage(u16 raster_x, u16 raster_y, u32 numPixel)
{
	if (popupMessage == nullptr) return;

	u16 lineaPixelesLetras = MARGEN_Y;
	u32 np;
	u8 letra;
	TColor* p;

	if (MARGEN_Y <= raster_y  &&  raster_y < MARGEN_Y + FONT_HEIGHT  
			&& popupMessage->len > 0  //&&  (raster_x & 1) == 0
			&&  MARGEN_X <= raster_x  &&  raster_x < MARGEN_X + popupMessage->pixelsLen)
	{
		//u8 filaCaracter = raster_y - MARGEN_Y;
		if (raster_x == MARGEN_X) {
			popupMessage->character = 0;
			popupMessage->chrCol = 0;
			popupMessage->chrRow = raster_y - MARGEN_Y;
			popupMessage->screenPixel = (MARGEN_Y+ 2*popupMessage->chrRow) * MONITOR_WIDTH + 2*MARGEN_X;
			popupMessage->chrPixel = font_getPtrChar(popupMessage->message[popupMessage->character]) + 
				popupMessage->chrRow * FONT_WIDTH;
			
		}

		if (*popupMessage->chrPixel) {
			#define p popupMessage->screenPixel
			//u32 p = osd_message.screenPixel;
			pixels[p] = pixels[p + 1] = pixels[p + MONITOR_WIDTH] = pixels[p + MONITOR_WIDTH + 1] = 
				colorValue(POPUP_MESSAGE_COLOR);
			#undef p
		}
		
		popupMessage->screenPixel += 2; // siguiente pixel en la pantalla
		popupMessage->chrPixel++; // siguiente pixel de la letra

		if (++popupMessage->chrCol == FONT_WIDTH) {
			popupMessage->chrCol = 0;
			popupMessage->character++;
			popupMessage->chrPixel = font_getPtrChar(popupMessage->message[popupMessage->character]) + 
				popupMessage->chrRow * FONT_WIDTH;
		}
	}
}



void OSD:: waitESC() {
	SDL_Keycode keycode;
	SDL_Event event;
	SDL_Scancode scancode;

    while (true) {
		SDL_PollEvent(&event);
		keycode = event.key.keysym.sym;
		scancode = SDL_GetScancodeFromKey(keycode);

		#ifdef TARGET_PC
		if (event.type == SDL_QUIT) {
			GLOBAL_QUIT = true;
			return;
		}
		#endif

		if (scancode == SDL_SCANCODE_ESCAPE) return;
	}
}




// invocar antes de visualizarla
void OSD:: push(OSD_Window& w) {
	u8 _col, _row, _width, _height;

	w.calculateSizePosition();
	w.getBounds(&_col, &_row, &_width, &_height);
	u16 col = _col;
	u16 row = _row;
	u16 width = _width;
	u16 height = _height;

	u32 pixel1 = getArrayIndex(getXchar(col), getYchar(row));  // pixel de la ventana
	pixel1 = pixel1 - 2 - 2*MONITOR_WIDTH;  // pixel del borde
	uint32_t* ptrFila = &pixels[pixel1];
	uint32_t* ptrColumna;

	width *= FONT_WIDTH * 2;  
	// cada pixel (mode1) del cpc lo representamos con 2x2 pixeles del pc (horz y vert), ademas tenemos el efecto scanline
	height *= FONT_HEIGHT * 2; 
	// cada pixel del cpc en mode2 lo representamos con 1x2 pixeles en el pc
	
	 // anadimos bordes (izq y der) y sombra
	width += 6;
	height += 6; 

	u32 size = width * height;   // los pixeles son de 2x1
	uint32_t* buffer = new uint32_t[size];
	OSD_WindowStack ws {pixel1, width, height, buffer};
	//debug_osd("OSD:: push %d,%d -> %d,%d  pixel=%d size=%d\n", col, row, width, height, pixel1, size);
	windowStack.push_back(ws);

	uint32_t* pixelBuffer = buffer;

	for (u16 y = 0; y < height; y++) {
		ptrColumna = ptrFila;
		for (u16 x = 0; x < width; x++, ptrColumna++) {
			*pixelBuffer++ = *ptrColumna;
		}
		ptrFila += MONITOR_WIDTH;
	}

	//{ // debug
	//	uint32_t pixel1 = ws.numPixel;
	//	uint32_t* ptrFila = &pixels[0];
	//	uint32_t* ptrColumna;
	//	uint32_t* ptrBuffer = ws.buffer;
	//
	//	for (u16 y = 0; y < ws.height; y++) {
	//		ptrColumna = ptrFila;
	//		for (u16 x = 0; x < ws.width; x++, ptrColumna += 2, ptrBuffer++) {
	//			*ptrColumna = *(ptrColumna+1) = (*ptrBuffer ^ 0xffffffff);
	//		}
	//		ptrFila += MONITOR_WIDTH;
	//	}
	//}

}

//---------------------------------------------------------------------------

void OSD:: _backStack() {
	//debug_osd("OSD:: pila%d\n", windowStack.size());
	if (windowStack.size() > 0) {
		OSD_WindowStack& ws = windowStack.back();

		//debug_osd("OSD:: back w,h = %d,%d  pixel=%d\n", ws.width, ws.height, ws.numPixel);
	
		uint32_t* ptrFila = &pixels[ws.numPixel];
		uint32_t* ptrColumna;
		uint32_t* ptrBuffer = ws.buffer;

		for (u16 y = 0; y < ws.height; y++) {
			ptrColumna = ptrFila;
			for (u16 x = 0; x < ws.width; x++, ptrColumna++, ptrBuffer++) {
				*ptrColumna = *ptrBuffer;
			}
			ptrFila += MONITOR_WIDTH;
		}
	}
}


void OSD:: backStack() {
	if (windowStack.size() > 0) {
		_backStack();
		updateUI();
	}
	//debug_osd("OSD:: back_stack  pila.size=%d\n", windowStack.size());
}


void OSD:: pop() {
	if (windowStack.size() > 0) {
		_backStack();
		windowStack.pop_back();
		updateUI();
	}
	//debug_osd("OSD:: back_stack  pila.size=%d\n", windowStack.size());
}