/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Types and constants                                                       |
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


typedef unsigned char      u8;
typedef unsigned short int u16;
typedef unsigned long      u32;
typedef unsigned long long u64;

typedef signed short int   i8;
typedef signed short int   i16;
typedef signed long        i32;
typedef signed long long   i64;


typedef unsigned short int DIR;  // para direcciones
typedef unsigned char      BYTE; // byte de 0 a 255
typedef signed char        SBYTE; // byte de -128 a 127


typedef union {
    struct {
        BYTE l, h;
    } b;
    u16 w;
} WORD;


typedef struct {
	u8 r, g, b;
} TColor;


constexpr BYTE BIT_7 = 0b10000000;
constexpr BYTE BIT_6 = 0b01000000;
constexpr BYTE BIT_5 = 0b00100000;
constexpr BYTE BIT_4 = 0b00010000;
constexpr BYTE BIT_3 = 0b00001000;
constexpr BYTE BIT_2 = 0b00000100;
constexpr BYTE BIT_1 = 0b00000010;
constexpr BYTE BIT_0 = 0b00000001;
