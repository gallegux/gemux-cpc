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

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h> // para IMG_SavePNG
#include "log.h"
#include "tipos.h"
#include "monitor.h"
#include "directories.h"
//#include "osd.h"
//#include "font.h"
#include "util.h"



constexpr u8 MASK_TIPO_ARRAY = 0b10000000;
constexpr u8 ARRAY_RGB	   	 = 0b00000000;
constexpr u8 ARRAY_NIVELES   = 0b10000000;

constexpr u8 MASK_NUM_ARRAY    = 0b00111000;	// numero de array de colores_rgb
constexpr u8 PAL_COLOR_TEORICO = 0b00000000;
constexpr u8 PAL_COLOR_REAL    = 0b00001000;
constexpr u8 NIVELES_1         = 0b00000000;
constexpr u8 NIVELES_2         = 0b00001000;

constexpr u8 DESPLZAMIENTO_ARRAY = 3  ;

constexpr u8 MASK_FUNCION = 0b00000111;
constexpr u8 FN_VERDE     = 0b00000000;
constexpr u8 FN_GRIS      = 0b00000001;
constexpr u8 FN_NARANJA   = 0b00000010;
constexpr u8 FN_RGB       = 0b00000011;


// codificamos las paletas del monitor - demasiao retorcio :/ cambiar
const BYTE DEFINICIONES_PALETA_MONITOR [4][4] = {
	// tipo_array, num.array en funcion del tipo_array, funcion_conversion
	{ ARRAY_RGB     | PAL_COLOR_TEORICO | FN_RGB,
	  ARRAY_RGB     | PAL_COLOR_REAL    | FN_RGB,
	  ARRAY_RGB     | PAL_COLOR_TEORICO | FN_RGB,
	  ARRAY_RGB     | PAL_COLOR_REAL    | FN_RGB },
	{ ARRAY_RGB     | PAL_COLOR_TEORICO | FN_VERDE,
	  ARRAY_RGB     | PAL_COLOR_REAL    | FN_VERDE,
	  ARRAY_NIVELES | NIVELES_1         | FN_VERDE,
	  ARRAY_NIVELES | NIVELES_2         | FN_VERDE },
	{ ARRAY_RGB     | PAL_COLOR_TEORICO | FN_GRIS,
	  ARRAY_RGB     | PAL_COLOR_REAL    | FN_GRIS,
	  ARRAY_NIVELES | NIVELES_1         | FN_GRIS,
	  ARRAY_NIVELES | NIVELES_2         | FN_GRIS },
	{ ARRAY_RGB     | PAL_COLOR_TEORICO | FN_NARANJA,
	  ARRAY_RGB     | PAL_COLOR_REAL    | FN_NARANJA,
	  ARRAY_NIVELES | NIVELES_1         | FN_NARANJA,
	  ARRAY_NIVELES | NIVELES_2         | FN_NARANJA }
};


const TColor* paletas_rgb[] = {paleta_color_teo, paleta_color_real};
const u8* paletas_nivel[] = {niveles_monocromo_1, niveles_monocromo_2};

const std::string PALETAS[] = {"Color", "Green", "Grays", "Orange"};




u32 getArrayIndex(u16 x, u16 y) {
	//x = x << 1;
	//y = y << 1;
	return y * MONITOR_WIDTH + x;
}


/*
void printOSD(uint32_t* pixels, u16 raster_x, u16 raster_y, u32 numPixel)
{
	u16 lineaPixelesLetras = MARGEN_Y;
	u32 np;
	u8 letra;
	TColor* p;

	if (MARGEN_Y <= raster_y  &&  raster_y < MARGEN_Y + FONT_HEIGHT  
			&& osd_message.len > 0  //&&  (raster_x & 1) == 0
			&&  MARGEN_X <= raster_x  &&  raster_x < MARGEN_X + osd_message.pixelsLen)
	{
		//u8 filaCaracter = raster_y - MARGEN_Y;
		if (raster_x == MARGEN_X) {
			osd_message.character = 0;
			osd_message.chrCol = 0;
			osd_message.chrRow = raster_y - MARGEN_Y;
			osd_message.screenPixel = (MARGEN_Y+ 2*osd_message.chrRow) * MONITOR_WIDTH + 2*MARGEN_X;
			osd_message.chrPixel = font_getPtrChar(osd_message.message[osd_message.character]) + 
				osd_message.chrRow * FONT_WIDTH;
			
		}

		if (*osd_message.chrPixel) {
			#define p osd_message.screenPixel
			//u32 p = osd_message.screenPixel;
			pixels[p] = pixels[p + 1] = pixels[p + MONITOR_WIDTH] = pixels[p + MONITOR_WIDTH + 1] = 
				osd_getColorMessage();
			#undef p
		}
		
		osd_message.screenPixel += 2; // siguiente pixel en la pantalla
		osd_message.chrPixel++; // siguiente pixel de la letra

		if (++osd_message.chrCol == FONT_WIDTH) {
			osd_message.chrCol = 0;
			osd_message.character++;
			osd_message.chrPixel = font_getPtrChar(osd_message.message[osd_message.character]) + 
				osd_message.chrRow * FONT_WIDTH;
		}
	}
}
*/


inline u8 mult(double v, double d) {
	return (u8) (v * d);
}

u8 luminancia(TColor c) {
	return mult(c.r, 0.2126) + mult(c.g, 0.7152) + mult(c.b, 0.0722);
}


TColor verde_l(u8 nivel) {
	TColor r;
	r.r = 0;
	r.g = nivel;
	r.b = 0;
	return r;
}

TColor verde_c(TColor c) { return verde_l( luminancia(c) ); }

TColor naranja_l(u8 nivel) {
	TColor r;
	r.r = nivel;
	r.g = nivel >> 1;
	r.b = 0;
	return r;
}

TColor naranja_c(TColor c) { return naranja_l( luminancia(c) ); }

TColor gris_l(u8 nivel) {
	TColor r;
	r.r = r.g = r.b = nivel;
	return r;
}

TColor gris_c(TColor c) { return gris_l( luminancia(c) ); }

TColor rgb_c(TColor c) { return c; }



Monitor:: Monitor() {
	debug_monitor("Monitor:: %dx%d\n", MONITOR_WIDTH, MONITOR_HEIGHT);
	//reset();
	/*
	SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Rojo
    SDL_RenderDrawLine(renderer, 0,0, MONITOR_WIDTH-1, MONITOR_HEIGHT-1);

	SDL_RenderPresent(renderer);  
	*/
	//paleta = paletas_monitor[PALETA_COLOR_REAL];
	numPaleta = COLOR_PALETTE;
	setPalette(numPaleta);

	scanlines = true;

	osd = nullptr;
}


void Monitor:: useScanlines(bool s) { scanlines = s; }

void Monitor:: flipScanlines() { scanlines = !scanlines; }

bool Monitor:: getScanlines() { return scanlines; }


//void Monitor:: reset() {
//	raster_x = 5;
//	raster_y = 0;
//
//	contadorActualizacion = LINEAS_ACTUALIZACION;
//
//}

//void Monitor:: setWindow(SDL_Window* window) {
//	this->window = window;
///*	for (int i = 0; i < MONITOR_WIDTH * MONITOR_HEIGHT) {
//		pixels[i] = SDL_MapRGB(SDL_GetWindowSurface(window)->format, 
//								i%255, (i/255)%255, (i/(255*255)%255));
//	}
//*/
//}
//void Monitor:: setRenderer(SDL_Renderer* renderer) {
//	this->renderer = renderer;
//}
//void Monitor:: setTexture(SDL_Texture* texture) {
//	this->texture = texture;
//}

void Monitor:: setOSD(OSD* _osd) {
	osd = _osd;
}

void Monitor:: setSDLObjects(SDL_Renderer* renderer, SDL_Texture* texture, uint32_t* pixels) {
	this->renderer = renderer;
	this->texture = texture;
	this->pixels = pixels;
}


void Monitor:: setPalette(u8 _numPaleta) {
	numPaleta = _numPaleta & 0x03;
	updatePalette();
}

void Monitor:: setPaletteVariant(u8 numVariant) {
	variantePaleta = numVariant & 0x03;
	updatePalette();
}

void Monitor:: setPalette(u8 _numPaleta, u8 _varPaleta) {
	numPaleta = _numPaleta & 0x03;
	variantePaleta = _varPaleta & 0x03;
	updatePalette();
}

std::string Monitor:: getPaletteName() { return PALETAS[numPaleta]; }

u8 Monitor:: getPalette() { return numPaleta; }

u8 Monitor:: getPaletteVariant() { return variantePaleta; }



void Monitor:: updatePalette() {
	debug_monitor("Monitor:: updatePalette()  num,var = %d,%d\n", numPaleta, variantePaleta);
	BYTE np = DEFINICIONES_PALETA_MONITOR[numPaleta][variantePaleta];
	BYTE numArray = (np & MASK_NUM_ARRAY) >> DESPLZAMIENTO_ARRAY;
	BYTE fn = np & MASK_FUNCION;
	//debug_monitor("Monitor:: np=%d array=%d fn=%d\n", np, numArray, fn);

	if ((np & MASK_TIPO_ARRAY) == ARRAY_RGB) {
		TColor (*funciones_conversion[4])(TColor) = {&verde_c, &gris_c, &naranja_c, &rgb_c};
		TColor (*funcion_conversion)(TColor) = funciones_conversion[fn];
		const TColor* p = paletas_rgb[numArray];

		for (u8 c = 0; c < 32; c++) {
			paleta[c] = funcion_conversion(p[c]);
			//debug_monitor("Monitor:: %d -> %d,%d,%d\n", 
			//	c, paleta[c].r, paleta[c].g, paleta[c].b);
		}
	}
	else {
		TColor (*funciones_conversion[3])(u8) = {&verde_l, &gris_l, &naranja_l};
		TColor (*funcion_conversion)(u8) = funciones_conversion[fn];
		const u8* niveles = paletas_nivel[numArray];

		for (u8 c = 0; c < 32; c++) {
			paleta[c] = funcion_conversion(niveles[c]);
			//debug_monitor("Monitor:: %d -> (%d) %d,%d,%d\n", 
			//	c, niveles[c], paleta[c].r, paleta[c].g, paleta[c].b);
		}
	}
}


//void monitor_setMensajePaleta(u8 numPaleta, u8 variantePaleta) {
//	osd_setMessage("Palette " + PALETAS[numPaleta] + "-" + std::to_string(variantePaleta+1));
//}


void Monitor:: changePalette() {
	printf("Monitor:: changePalette()");
	numPaleta++;
	numPaleta = numPaleta & 0x03;
	//if (numPaleta >= NUM_PALETAS_MONITOR) numPaleta -= NUM_PALETAS_MONITOR;
	updatePalette();
	//monitor_setMensajePaleta(numPaleta, variantePaleta);
}

void Monitor:: changePaletteVariant() {
	printf("Monitor:: cambiarVarPaleta()");
	variantePaleta++;
	variantePaleta = variantePaleta & 0x03;
	updatePalette();
	//monitor_setMensajePaleta(numPaleta, variantePaleta);
}

/** raster_y va desde vdur_min **/
void Monitor:: printPixel(u8 colorHw) {
	TColor c = paleta[colorHw];
	//debug("<CH=%02X=%d,%d,%d> ", colorHw, c.r, c.g, c.b);
	//SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
	//SDL_RenderDrawPoint(renderer, raster_x, y);
	//SDL_RenderDrawPoint(renderer, raster_x, ++y); 
	if (raster_x < MAX_RASTER_X  &&  raster_y >= 0  &&  raster_y < LINEAS_VISIBLES*8) { //MAX_RASTER_Y-MIN_RASTER_Y) {
	//if (raster_x < MAX_RASTER_X  &&  raster_y >= -8  &&  raster_y < -8+LINEAS_VISIBLES*8) { //MAX_RASTER_Y-MIN_RASTER_Y) {
		uint32_t numPixel = (raster_y << 1) * MONITOR_WIDTH + raster_x;
		
		//if (numPixel == numPixelIzq+1) {
		//	raster_x++;
		//	return;
		//}

		pixels[numPixel] = c.r << 16 | c.g << 8 | c.b;
		//pixels[numPixel] = //pixels[numPixel + MONITOR_WIDTH] = 
		//	SDL_MapRGB(SDL_GetWindowSurface(window)->format, c.r, c.g, c.b);

		if (scanlines) {
			pixels[numPixel + MONITOR_WIDTH] = (c.r>>1)<<16 | (c.g>>1)<<8 | c.b>>1;
			//pixels[numPixel + MONITOR_WIDTH] = SDL_MapRGB(SDL_GetWindowSurface(window)->format, c.r>>1, c.g>>1, c.b>>1);
		}
		else {
			pixels[numPixel + MONITOR_WIDTH] = pixels[numPixel];
		}
		//printOSD(pixels, raster_x, raster_y, numPixel);
		if (osd != nullptr) osd->printPopupMessage(raster_x, raster_y, numPixel);
	}
	//if (raster_x < MAX_RASTER_X  &&  raster_y >= MIN_RASTER_Y  &&  raster_y < MAX_RASTER_Y) {
	//	uint32_t numPixel = (raster_y << 1) * MONITOR_WIDTH + raster_x;
	//	
	//	pixels[numPixel] = pixels[numPixel + MONITOR_WIDTH] = 
	//		SDL_MapRGB(SDL_GetWindowSurface(window)->format, c.r, c.g, c.b);
	//}
	raster_x++;

}


// se reciben colores hardware
void Monitor:: update(u8* arrayTintas) {
	for (u8 i = 0; i < 8; i++)  printPixel(arrayTintas[i]);
	//SDL_RenderPresent(renderer);  
}

void Monitor:: update(bool hsync, bool vsync) {
	raster_x += 8;
	//bool c;
	//for (u8 i = 0; i < 8; i++) {
	//	if (hsync) printPixel(0x40); else printPixel(0x42);
	//	if (vsync) printPixel(0x41); else printPixel(0x43);
	//}
	//SDL_RenderPresent(renderer);  
}

void Monitor:: update(u8 colorHwBorde) {
	for (u8 i = 0; i < 16; i++)  printPixel(colorHwBorde);
	//SDL_RenderPresent(renderer);  
}


void Monitor:: nextLine() {
	if (!--contadorActualizacion) {
		SDL_RenderPresent(renderer);  
		contadorActualizacion = LINEAS_ACTUALIZACION;
	}
	raster_x = 0;
	raster_y++;
}

void Monitor:: nextFrame(i16 vdur_min) {
//	debug("next frame %d\n", raster_y);

	// update ted texture with the pixels array
	SDL_UpdateTexture(texture, NULL, pixels, MONITOR_WIDTH*sizeof(uint32_t));
	// render the texture to the window
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

	raster_y = vdur_min+8;// -MIN_RASTER_Y;
}


bool Monitor:: saveScreen(const std::string& fichero) {
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);
    
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        std::cerr << "Failed to create surface: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_SetRenderTarget(renderer, texture);
    SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32, surface->pixels, surface->pitch);
    SDL_SetRenderTarget(renderer, NULL);

	int resultadoGrabar = IMG_SavePNG(surface, fichero.c_str());

    SDL_FreeSurface(surface);

	return (resultadoGrabar == 0);
}



