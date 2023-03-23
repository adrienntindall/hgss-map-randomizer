#ifndef HGSS_ROM_HANDLER_H
#define HGSS_ROM_HANDLER_H

#include <string>

using std::string;

#define ARM9_DEC    "tools/blz.exe"
#define W2_PATH     "white2_rom/"
#define B2_PATH     "black2_rom/"
#define OUT_PATH    "randomized_roms/"
#define W2_DATA     "./files/Blocks_W2/"
#define B2_DATA     "./files/Blocks_B2/"
#define BW2_DATA_SHARED  "./files/Blocks_BW2_Shared/"
#define SPRINGSUMMER_DATA "./files/Blocks_BW2_Seasons/SpringSummer/"
#define AUTUMN_DATA "./files/Blocks_BW2_Seasons/Autumn/"
#define WINTER_DATA "./files/Blocks_BW2_Seasons/Winter/"
#define NDS_TOOL    "tools/ndstool.exe"
#define KNARC       "tools/knarc.exe"   
#define EVENT_DATA  "files/bw2_changes/event_data_bw2" 
#define TRAINER_DATA "files/bw2_changes/tr_data_bw2"
#define TRAINER_POKE "files/bw2_changes/tr_poke_bw2"
#define MAP_FUNCTIONS "files/bw2_changes/map_functions_bw2"
#define OW_SPRITES   "files/bw2_changes/ow_sprites"
#define MUGSHOTS     "files/bw2_changes/mugshots"

//Season consts
enum Seasons {
    SEASON_SPRING,
    SEASON_SUMMER,
    SEASON_AUTUMN,
    SEASON_WINTER
};

//File change locations
#define BW2_UNIVERSAL       "files/bw2_script_universal.csv"
#define SEASON_LOCK_BASE_W2 "files/Blocks_BW2_Seasons/SeasonScripts/W2SeasonLock"
#define SEASON_LOCK_BASE_B2 "files/Blocks_BW2_Seasons/SeasonScripts/B2SeasonLock"

void LockSeason(string changesPath, const char season);
void UnpackRom(string path, string arm9);
void UnpackFieldNarc();
void PackFieldNarc();
void RepackRom(string out);
void ClearTempData();

#endif //HGSS_ROM_HANDLER_H
