#include "RomHandler.h"
#include <iostream>
#include <filesystem>
#include <fstream>

using namespace std;
using std::filesystem::directory_iterator;
using std::filesystem::recursive_directory_iterator;

static string langExt = "_english.csv";

inline string csvGotoNext(string str) {
    return str.substr(str.find(',') + 1, str.length());
}

inline string csvGetNext(string str) {
    return str.substr(0, str.find(','));
}

void ApplyChanges(string changesPath) {
    changesPath += langExt;
    
    fstream bin;
    bin.open("temp\\arm9.bin", ios::binary|ios::in|ios::out|ios::ate);
    
    ifstream changes;
    changes.open(changesPath);
    
    string line;
    
    getline(changes, line);
    
    while (getline(changes, line)) {
        int pos = stoi(csvGetNext(line));
        line = csvGotoNext(line);
        char dat = stoi(line);
        bin.seekp(pos);
        bin.write(&dat, 1);
    }
    
    changes.close();
    bin.close();
}

void LockSeason(string changesPath, const char season) {
    changesPath += langExt;
    
    fstream bin;
    bin.open("temp\\arm9.bin", ios::binary|ios::in|ios::out|ios::ate);
    
    ifstream changes;
    changes.open(changesPath);
    
    string line;
    
    getline(changes, line);
    
    bool flag = false;
    
    while (getline(changes, line)) {
        int pos = stoi(csvGetNext(line));
        line = csvGotoNext(line);
        char dat = stoi(line);
        bin.seekp(pos);
        if(flag) {
            bin.write(&dat, 1);
        }
        else {
            bin.write(&season, 1);
            flag = true;
        }
    }
    
    changes.close();
    bin.close();
}

void UnpackRom(string romPath, string arm9) {
    string command = string(NDS_TOOL) + " -x \"" + romPath + "\" -9 temp\\arm9.bin -7 temp\\arm7.bin -y9 temp\\y9.bin -y7 temp\\y7.bin -d data -y temp\\overlay -t temp\\banner.bin -h temp\\header.bin";
    cout << command << endl;
    system(command.c_str());
    
    command = string(ARM9_DEC) + " -d temp\\arm9.bin";
    system(command.c_str());
    
    ApplyChanges(string(BW2_UNIVERSAL));
    
    fstream bin;
    bin.open("temp\\arm9.bin", ios::binary|ios::in|ios::out|ios::ate);
    
    unsigned char lang;
    
    bin.seekp(0xE); //I don't know where the language flag actually is in this game so a random bit will have to work lol
    
    bin.read((char*) &lang, 2);
    
    switch(lang) {
        case 62: //W2
        case 35: //B2
            cout << "English rom detected" << endl;
            langExt = "_english.csv";
            break;
        case 227: //W2
        case 171: //B2
            cout << "Deutsche ROM erkannt" << endl;
            langExt = "_german.csv";
            break;
        default:
            cout << "WARNING: Detected language not recognized, either your language is not supported or there may be an error with your rom" << endl;
            cout << "Detected language ID: " << int(lang) << endl;
            
            break;
    }
    
    bin.close();
}

inline void CopyFiles(string narc_path, string temp_path, string data_path) {
    cout << data_path << endl;
    string command = string(KNARC) + " -d " + temp_path + " -u " + narc_path;
    system(command.c_str());
    
    command = "copy " + string(data_path);
    command += " " + temp_path;
    system(command.c_str());
    
    command = string(KNARC) + " -d " + temp_path + " -p " + narc_path;
    system(command.c_str());
}

void UnpackFieldNarc() {
    system("echo off");
    
    //Updating some select files:
    //Event Data
    CopyFiles("data\\a\\1\\2\\6", "temp\\event_data", EVENT_DATA);
    
    //Trainer Data
    CopyFiles("data\\a\\0\\9\\1", "temp\\trainer_data", TRAINER_DATA);

    //Trainer Poke
    CopyFiles("data\\a\\0\\9\\2", "temp\\trainer_poke", TRAINER_POKE);
    
    //Map Scripts
    CopyFiles("data\\a\\0\\5\\6", "temp\\map_script", MAP_FUNCTIONS);
    
    //Map headers
    string command = string(KNARC) + " -d temp\\map_header -u data\\a\\0\\1\\2";
    system(command.c_str());
    
    for(const auto & file : directory_iterator("temp\\map_header")) {
        fstream bin;
        bin.open(file.path(), ios::binary|ios::in|ios::out|ios::ate);
        bin.seekg(0, ios::end);
        
        streampos size = bin.tellg();
        
        bin.seekp(0);
        
        vector<unsigned char> binData(size);
        bin.read((char*) &binData[0], size);
        
        unsigned int pos = 0x1F;
        
        while(pos < size) {
            bin.seekp(pos);
            binData[pos] = binData[pos] | 0x20; //sets the fly flag, if unset
            bin.write((char*) &binData[pos], 1); 
            pos += 0x30;
        }
        bin.close();
    }
    
    command = string(KNARC) + " -d temp\\map_header -p data\\a\\0\\1\\2";
    system(command.c_str());
    
    //=)
    CopyFiles("data\\a\\0\\4\\8", "temp\\ow_spr", OW_SPRITES);
    system("echo on");
}

void PackFieldNarc() {
    string command = string(KNARC) + " -d temp\\event_data -p data\\a\\1\\2\\6";
    system(command.c_str());
}

void RepackRom(string output_name) {
    cout << output_name << endl;
    string command = string(NDS_TOOL) + " -c " + "\"" + string(OUT_PATH) + output_name + "\"" + " -9 temp\\arm9.bin -7 temp\\arm7.bin -y9 temp\\y9.bin -y7 temp\\y7.bin -d data -y temp\\overlay -t temp\\banner.bin -h temp\\header.bin";
    system(command.c_str());
}

static void deleteDirectoryContents(const std::string& dir_path)
{
    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) 
        std::filesystem::remove_all(entry.path());
}

void ClearTempData() {
    deleteDirectoryContents("./temp");
    deleteDirectoryContents("./data");
}
