#include "RomHandler.h"
#include <iostream>
#include <filesystem>
#include <fstream>

using namespace std;
using std::filesystem::directory_iterator;
using std::filesystem::recursive_directory_iterator;

inline string csvGotoNext(string str) {
    return str.substr(str.find(',') + 1, str.length());
}

inline string csvGetNext(string str) {
    return str.substr(0, str.find(','));
}

void UnpackRom(string romPath, string arm9) {
    string command = string(NDS_TOOL) + " -x \"" + romPath + "\" -9 temp\\arm9.bin -7 temp\\arm7.bin -y9 temp\\y9.bin -y7 temp\\y7.bin -d data -y temp\\overlay -t temp\\banner.bin -h temp\\header.bin";
    cout << command << endl;
    system(command.c_str());
    
    //Updating some of the files...
    
    command = string(ARM9_DEC) + " temp\\arm9.bin " + " temp\\arm9_.bin";
    
    system(command.c_str());
    
    fstream bin;
    bin.open("temp\\arm9_.bin", ios::binary|ios::in|ios::out|ios::ate);
    
    unsigned char lang = 2;
    
    bin.seekp(0xF5670);
    
    //bin.read(&lang, 1);
    ifstream changes;
    
    switch(lang) {
    case 2: //English
        changes.open(arm9);
        break;
    case 5: //German
        changes.open(arm9 + "_german");
        break;
    default:
        cout << "WARNING: Detected language not currently supported, so the English scripts will be used. This may result in an unloadable game." << endl;
        changes.open(arm9);
        break;
    }
    
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
    
    command = "copy files\\scr_seq.narc data\\a\\0\\1\\2";
    system(command.c_str());
    
}

void UnpackFieldNarc() {
    string command = string(KNARC) + " -d temp\\event_data -u data\\a\\0\\3\\2";
    system(command.c_str());
    
    //Updating some select files:
    for(const auto & file : directory_iterator(EVENT_DATA)) {
        string pstr = file.path().string();
        command = "copy " + file.path().string();
        pstr = pstr.substr(pstr.find_last_of("\\") + 1, pstr.length());
        command += " temp\\event_data\\" + pstr;
        system(command.c_str());
    }
}

void PackFieldNarc() {
    string command = string(KNARC) + " -d temp\\event_data -p data\\a\\0\\3\\2";
    system(command.c_str());
}

void RepackRom(string output_name) {
    cout << output_name << endl;
    string command = string(NDS_TOOL) + " -c " + "\"" + string(OUT_PATH) + output_name + "\"" + " -9 temp\\arm9_.bin -7 temp\\arm7.bin -y9 temp\\y9.bin -y7 temp\\y7.bin -d data -y temp\\overlay -t temp\\banner.bin -h temp\\header.bin";
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
