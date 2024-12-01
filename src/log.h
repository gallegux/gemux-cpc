#pragma once

#include <iostream>
#include <iomanip>
#include "compilation_options.h"

#ifdef PRINT_DEBUG
#define debug printf
#define FLUSH std::cout << std::flush;
#else
#define debug //printf
#define FLUSH //std::cout << std::flush;
#endif


#define debug_cpc //debug
#define debug_z80 //debug
#define debug_io //debug
#define debug_crtc //debug
#define debug_ppi debug
#define debug_psg //debug
#define debug_ga //debug
#define debug_disquetera //debug
#define debug_monitor //debug
#define debug_osd //debug
#define debug_fdc //debug
#define debug_dsk //debug
#define debug_mem //debug
#define debug_tape debug
#define debug_cdt debug
#define debug_kb //debug
#define debug_sna //debug
#define debug_z debug
