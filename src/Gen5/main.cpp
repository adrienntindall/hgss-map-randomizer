#include <algorithm>
#include <cstdlib>
#include <random>
#include <filesystem>
#include <iostream>
#include <functional>
#include <time.h>
#include "Randomizer.h"
#include "RomHandler.h"

using namespace std;
using std::filesystem::directory_iterator;
using std::filesystem::recursive_directory_iterator;

static bool flag = false;
static mt19937 generator;

static unsigned long int getRandomSeed() {
    return generator();
}

static long int getSeed(string* seedInput){
    while(true){
        cout << "Enter a seed number (blank for random): ";
        getline(cin, *seedInput);

        if(seedInput->empty()){
            return getRandomSeed();
        } else {
            const char* start = seedInput->c_str();
            char* end = (char*)start;

            long int seed = strtol(start, &end, 10);

            if(start != end){
                return seed;
            }
            hash<string> str_hash;
            return str_hash(*seedInput);
        }
    }
}

static int getSeason(string* seasonInput){
    while(true){
        cout << "Enter a season (blank for random, 'current' for current season): ";
        getline(cin, *seasonInput);
        transform(seasonInput->begin(), seasonInput->end(), seasonInput->begin(), ::tolower);
        if(seasonInput->empty()){
            return rand() % 4;
        } else {
            if(*seasonInput == "autumn" || *seasonInput == "fall" || *seasonInput == "2" ||
                *seasonInput == "herbst") return SEASON_AUTUMN;
            if(*seasonInput == "spring" || *seasonInput == "0" ||
                *seasonInput == "frühling" || *seasonInput == "fruhling") return SEASON_SPRING;
            if(*seasonInput == "summer" || *seasonInput == "1" ||
                *seasonInput == "sommer") return SEASON_SUMMER;
            if(*seasonInput == "winter") return SEASON_WINTER;
            if(*seasonInput == "current" || *seasonInput == "3" || *seasonInput == "aktuell") {
                time_t t = time(NULL);
                tm* now = localtime(&t);
                return now->tm_mon % 4;
            }
            cout << "Invalid season!" << endl;
        }
    }
}

static void HandleRandomization(string directory, string data, string season_path) {
    generator = mt19937(time(NULL));
    srand(time(NULL));
    for(const auto & file : recursive_directory_iterator(directory)) {
        flag = true;
        string pstr = file.path().string();
        UnpackRom(pstr, "");
        UnpackFieldNarc();
        string seedInput;
        string seasonInput;
        long int seed = getSeed(&seedInput);
        int season = getSeason(&seasonInput);
        while(true) {
            if (seedInput.empty()){
                seed = (long int) time(NULL);
            }
            cout << "Trying seed " << seed << endl;
            GetWarpList();
            GetWarpDict();
            GetBlocks(string(data));
            GetBlocks(string(BW2_DATA_SHARED));
            switch(season) {
                case SEASON_SPRING:
                case SEASON_SUMMER:
                    GetBlocks(string(SPRINGSUMMER_DATA));
                    break;
                case SEASON_AUTUMN:
                    GetBlocks(string(AUTUMN_DATA));
                    break;
                case SEASON_WINTER:
                    GetBlocks(string(WINTER_DATA));
                    break;
            }
            if(RandomizeMap(seed)) break;
            cout << "Error: Unbeatable seed. Retrying..." << endl;
            if (!seedInput.empty()){
                seed = getSeed(&seedInput);
            }
            else {
                seed = getRandomSeed();
            }
            if(!seasonInput.empty()) {
                season = getSeason(&seasonInput);
            }
            else {
                season = rand() % 4;
            }
            ClearData();
        }
        LockSeason(season_path, season);
        pstr = pstr.substr(pstr.find_last_of("/") + 1, pstr.length()-4) + "_map_randomized";
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
    HandleRandomization(W2_PATH, W2_DATA, string(SEASON_LOCK_BASE_W2));
    //B2
    HandleRandomization(B2_PATH, B2_DATA, string(SEASON_LOCK_BASE_B2));
    if(!flag) {
        cout << "No rom detected, please refer to HowToUse.txt" << endl;
    }
    system("pause");
}
