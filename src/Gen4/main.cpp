#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <time.h>
#include "Randomizer.h"
#include "RomHandler.h"

using namespace std;
using std::filesystem::directory_iterator;
using std::filesystem::recursive_directory_iterator;

static bool flag = false;

static long int getSeed(string* seedInput){
    while(true){
        cout << "Enter seed (blank for default): ";
        getline(cin, *seedInput);

        if(seedInput->empty()){
            return (long int) time(NULL);
        } else {
            const char* start = seedInput->c_str();
            char* end = (char*)start;

            long int seed = strtol(start, &end, 10);

            if(start != end){
                return seed;
            }
        }
    }
}

static void HandleRandomization(string directory, string data, string arm9) {
    for(const auto & file : recursive_directory_iterator(directory)) {
        flag = true;
        string pstr = file.path().string();
        UnpackRom(pstr, arm9);
        UnpackFieldNarc();
        string seedInput;
        long int seed = getSeed(&seedInput);
        while(true) {
            if (seedInput.empty()){
                seed = (long int) time(NULL);
            }
            cout << "Trying seed " << seed << endl;
            GetWarpList();
            GetWarpDict();
            GetBlocks(HGSS_COMMON);
            GetBlocks(string(data));
            if(RandomizeMap(seed)) break;
            cout << "Error: Unbeatable seed. Retrying..." << endl;
            if (!seedInput.empty()){
                seed = getSeed(&seedInput);
            }
            ClearData();
        }
        pstr = pstr.substr(pstr.find_last_of("\\") + 1, pstr.length()-4) + "_map_randomized";
        GenerateLogFile(OUT_PATH + pstr + ".log");
        SetWarps();
        PackFieldNarc();
        RepackRom(pstr + ".nds");
        ClearTempData();
    }
}

int main() {
    ClearTempData();
    //Soul Silver
    HandleRandomization(SS_PATH, SS_DATA, SS_ARM9);
    //Heart Gold
    HandleRandomization(HG_PATH, HG_DATA, HG_ARM9);
    if(!flag) {
        cout << "No rom detected, please refer to HowToUse.txt" << endl;
    }
    system("pause");
}
