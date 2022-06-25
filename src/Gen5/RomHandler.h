#ifndef HGSS_ROM_HANDLER_H
#define HGSS_ROM_HANDLER_H

#include <string>

using std::string;

#define W2_PATH     "white2_rom\\"
#define OUT_PATH    "randomized_roms\\"
#define W2_DATA     "./files/Blocks_W2/"
#define NDS_TOOL    "tools\\ndstool.exe"
#define KNARC       "tools\\knarc.exe"   
#define EVENT_DATA  "files\\bw2_changes\\event_data_bw2" 
#define TRAINER_DATA "files\\bw2_changes\\tr_data_bw2"
#define TRAINER_POKE "files\\bw2_changes\\tr_poke_bw2"
#define MAP_FUNCTIONS "files\\bw2_changes\\map_functions_bw2"
#define OW_SPRITES   "files\\bw2_changes\\ow_sprites"
#define MUGSHOTS     "files\\bw2_changes\\mugshots"

void UnpackRom(string path, string arm9);
void UnpackFieldNarc();
void PackFieldNarc();
void RepackRom(string out);
void ClearTempData();

#endif //HGSS_ROM_HANDLER_H
