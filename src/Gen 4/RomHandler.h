#ifndef HGSS_ROM_HANDLER_H
#define HGSS_ROM_HANDLER_H

#include <string>

using std::string;

#define SS_PATH     "./soulsilver_rom/"
#define SS_ARM9     "./files/hgss_script.csv"
#define HG_ARM9     "./files/hgss_script.csv"
#define HG_PATH     "./heartgold_rom/"
#define OUT_PATH    "./randomized_roms/"
#define HG_DATA     "./files/Blocks_HG/"
#define SS_DATA     "./files/Blocks_SS/"
#define NDS_TOOL    "./tools/ndstool.exe"
#define KNARC       "./tools/knarc.exe"
#define ARM9_DEC    "./tools/arm9dec.exe"
#define EVENT_DATA  "./files/event_data"

void UnpackRom(const string& path, const string& arm9);
void UnpackFieldNarc();
void PackFieldNarc();
void RepackRom(const string& out);
void ClearTempData();

#endif //HGSS_ROM_HANDLER_H
