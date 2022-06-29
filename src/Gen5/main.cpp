#include <cstdlib>
#include <filesystem>
#include <iostream>
#include "Randomizer.h"
#include "RomHandler.h"

using namespace std;
using std::filesystem::directory_iterator;
using std::filesystem::recursive_directory_iterator;

static bool flag = false;

static long int getSeed(string* seedInput){
    while(true){
        cout << "Enter a seed number (blank for default): ";
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
	    cout << "Seed malformed!" << endl;
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
    //W2
    HandleRandomization(W2_PATH, W2_DATA, "");
    if(!flag) {
        cout << "No rom detected, please refer to HowToUse.txt" << endl;
    }
    system("pause");
}
