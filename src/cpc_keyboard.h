/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  CPC Keyboard                                                              |
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


// linea 40
constexpr u16 CPC_TN_PUNTO        = 0x4080;
constexpr u16 CPC_INTRO           = 0x4040;
constexpr u16 CPC_F3              = 0x4020;
constexpr u16 CPC_F6              = 0x4010;
constexpr u16 CPC_F9              = 0x4008;
constexpr u16 CPC_CURSOR_ABAJO    = 0x4004;
constexpr u16 CPC_CURSOR_DERECHA  = 0x4002;
constexpr u16 CPC_CURSOR_ARRIBA   = 0x4001;
// linea 41
constexpr u16 CPC_F0          = 0x4180;
constexpr u16 CPC_F2          = 0x4140;
constexpr u16 CPC_F1          = 0x4120;
constexpr u16 CPC_F5          = 0x4110;
constexpr u16 CPC_F8          = 0x4108;
constexpr u16 CPC_F7          = 0x4104;
constexpr u16 CPC_COPIA       = 0x4102;
constexpr u16 CPC_CURSOR_IZQ  = 0x4101;
// linea 42
constexpr u16 CPC_CONTROL     = 0x4280;
constexpr u16 CPC_BARRA_INV   = 0x4240;
constexpr u16 CPC_MAYUSCULAS  = 0x4220;
constexpr u16 CPC_F4          = 0x4210;
constexpr u16 CPC_MAS         = 0x4208;
constexpr u16 CPC_RETURN      = 0x4204;
constexpr u16 CPC_ASTERISCO   = 0x4202;
constexpr u16 CPC_CLR         = 0x4201;
// linea 43
constexpr u16 CPC_PUNTO       = 0x4380;
constexpr u16 CPC_BARRA       = 0x4340;
constexpr u16 CPC_ENYE        = 0x4320;
constexpr u16 CPC_DOS_PUNTOS  = 0x4310;
constexpr u16 CPC_P           = 0x4308;
constexpr u16 CPC_ARROBA      = 0x4304;
constexpr u16 CPC_MENOS       = 0x4302;
constexpr u16 CPC_PESETAS     = 0x4301;
// linea 44
constexpr u16 CPC_COMA        = 0x4480;
constexpr u16 CPC_M           = 0x4440;
constexpr u16 CPC_K           = 0x4420;
constexpr u16 CPC_L           = 0x4410;
constexpr u16 CPC_I           = 0x4408;
constexpr u16 CPC_O           = 0x4404;
constexpr u16 CPC_9           = 0x4402;
constexpr u16 CPC_0           = 0x4401;
// linea 45
constexpr u16 CPC_ESPACIO     = 0x4580;
constexpr u16 CPC_N           = 0x4540;
constexpr u16 CPC_J           = 0x4520;
constexpr u16 CPC_H           = 0x4510;
constexpr u16 CPC_Y           = 0x4508;
constexpr u16 CPC_U           = 0x4504;
constexpr u16 CPC_7           = 0x4502;
constexpr u16 CPC_8           = 0x4501;
// linea 46
constexpr u16 CPC_V           = 0x4680;
constexpr u16 CPC_B           = 0x4640;
constexpr u16 CPC_F           = 0x4620;
constexpr u16 CPC_G           = 0x4610;
constexpr u16 CPC_T           = 0x4608;
constexpr u16 CPC_R           = 0x4604;
constexpr u16 CPC_5           = 0x4602;
constexpr u16 CPC_6           = 0x4601;
// linea 47
constexpr u16 CPC_X           = 0x4780;
constexpr u16 CPC_C           = 0x4740;
constexpr u16 CPC_D           = 0x4720;
constexpr u16 CPC_S           = 0x4710;
constexpr u16 CPC_W           = 0x4708;
constexpr u16 CPC_E           = 0x4704;
constexpr u16 CPC_3           = 0x4702;
constexpr u16 CPC_4           = 0x4701;
// linea 48
constexpr u16 CPC_Z           = 0x4880;
constexpr u16 CPC_FIJA_MAYS   = 0x4840;
constexpr u16 CPC_A           = 0x4820;
constexpr u16 CPC_TAB         = 0x4810;
constexpr u16 CPC_Q           = 0x4808;
constexpr u16 CPC_ESC         = 0x4804;
constexpr u16 CPC_2           = 0x4802;
constexpr u16 CPC_1           = 0x4801;
// linea 49
constexpr u16 CPC_BORR        = 0x4980;
constexpr u16 CPC_JOY1_FIRE3  = 0x4940;
constexpr u16 CPC_JOY1_FIRE2  = 0x4920;
constexpr u16 CPC_JOY1_FIRE1  = 0x4910;
constexpr u16 CPC_JOY1_DER    = 0x4908;
constexpr u16 CPC_JOY1_IZQ    = 0x4904;
constexpr u16 CPC_JOY1_ABAJO  = 0x4902;
constexpr u16 CPC_JOY1_ARRIBA = 0x4901;
