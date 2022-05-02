#include "RomHandler.h"
#include <iostream>
#include <filesystem>
#include <fstream>

using namespace std;
using std::filesystem::directory_iterator;
using std::filesystem::recursive_directory_iterator;

void UnpackRom(string romPath, string arm9) {
    string command = string(NDS_TOOL) + " -x \"" + romPath + "\" -9 temp\\arm9.bin -7 temp\\arm7.bin -y9 temp\\y9.bin -y7 temp\\y7.bin -d data -y temp\\overlay -t temp\\banner.bin -h temp\\header.bin";
    cout << command << endl;
    system(command.c_str());
    
    //Updating some of the files...
    command = "cp " + arm9 + " temp\\arm9.bin";
    system(command.c_str());
    
    command = "cp files\\scr_seq.narc data\\a\\0\\1\\2";
    system(command.c_str());
    
}

void UnpackFieldNarc() {
    string command = string(KNARC) + " -d temp\\event_data -u data\\a\\0\\3\\2";
    system(command.c_str());
    
    //Updating some select files:
    for(const auto & file : directory_iterator(EVENT_DATA)) {
        string pstr = file.path().string();
        command = "cp " + file.path().string();
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
