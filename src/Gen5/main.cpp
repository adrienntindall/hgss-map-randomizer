#include <cstdlib>
#include <filesystem>
#include <iostream>
#include "Randomizer.h"
#include "RomHandler.h"

using namespace std;
using std::filesystem::directory_iterator;
using std::filesystem::recursive_directory_iterator;

static bool flag = false;

static void HandleRandomization(string directory, string data, string arm9) {
    for(const auto & file : recursive_directory_iterator(directory)) {
        flag = true;
        string pstr = file.path().string();
        UnpackRom(pstr, arm9);
        UnpackFieldNarc();
        while(true) {
            GetWarpList();
            GetWarpDict();
            GetBlocks(string(data));
            if(RandomizeMap()) break;
            cout << "Error: Unbeatable seed. Retrying..." << endl;
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
    //W2
    HandleRandomization(W2_PATH, W2_DATA, "");
    if(!flag) {
        cout << "No rom detected, please refer to HowToUse.txt" << endl;
    }
    system("pause");
}
