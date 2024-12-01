#pragma once


#include <string>
#include <string_view>

//#define ROMS_DIR "roms/"
constexpr std::string_view MACHINES_DIR = "machines";
constexpr std::string_view ROMS_DIR = "roms";
constexpr std::string_view DISKS_DIR = "disks";
constexpr std::string_view TAPES_DIR = "tapes";
constexpr std::string_view SNAPSHOTS_DIR = "snapshots";
constexpr std::string_view SCREENSHOTS_DIR = "screenshots";

constexpr std::string_view MACHINES_EXT = ".cpc";
constexpr std::string_view ROMS_EXT = ".rom";
constexpr std::string_view DISK_EXT = ".dsk";
constexpr std::string_view TAPE_EXT = ".cdt";
constexpr std::string_view SNAPSHOT_EXT = ".sna";
constexpr std::string_view SCREENSHOT_EXT = ".png";

constexpr std::string_view CSW_AUX_FILE = "tapes/csw.tmp";
constexpr std::string_view LAST_CONFIGURATION_FILE = "machines/last_config.cpc";
constexpr std::string_view WINDOW_POSITION_FILE = "window.cfg";
