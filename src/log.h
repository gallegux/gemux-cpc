#pragma once

#include <iostream>
#include <iomanip>


//#define G printf
#define debug printf

#define debug_z80 //printf
#define debug_crtc //printf
#define debug_ppi //printf
#define debug_psg printf
#define debug_teclado //printf
#define debug_ga //printf
#define debug_monitor printf
#define debug_fdc printf
#define debug_io //printf
#define debug_mem //printf
#define debug_dsk printf
#define debug_disquetera printf
#define debug_cdt //printf
#define debug_cinta //printf
#define debug_tape //printf



#define FLUSH std::cout << std::flush;


//std::string toHex(u32 numero, u8 caracteres) {
//    std::ostringstream stream;
//    stream << std::hex << std::setw(caracteres) << std::setfill('0') << numero;
//    return stream.str();
//}
//
//std::string toHex2(u32 numero) {
//    std::ostringstream stream;
//    stream << std::hex << std::setw(2) << std::setfill('0') << numero;
//    return stream.str();
//}
//
//std::string toHex4(u32 numero) {
//    std::ostringstream stream;
//    stream << std::hex << std::setw(4) << std::setfill('0') << numero;
//    return stream.str();
//}
//
//
//std::string toHex5(u32 numero) {
//    std::ostringstream stream;
//    stream << std::hex << std::setw(5) << std::setfill('0') << numero;
//    return stream.str();
//}


//extern bool log_verbose;

#define LOG_TO(stream,level,message) stream << (level) << " " << __FILE__ << ":" << __LINE__ << " - " << message << std::endl; // NOLINT(misc-macro-parentheses): Not having parentheses around message is a feature, it allows using streams in LOG macros

#define LOG_ERROR(message)   LOG_TO(std::cerr, "ERROR  ", message)
#define LOG_WARNING(message) LOG_TO(std::cerr, "WARNING", message)
#define LOG_INFO(message)    LOG_TO(std::cerr, "INFO   ", message)
#define LOG_DEBUG(message)   LOG_TO(std::cout, "DEBUG  ", message)

