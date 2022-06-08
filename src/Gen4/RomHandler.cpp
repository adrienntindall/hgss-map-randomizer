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
    
    bin.read((char*) &lang, 1);
    ifstream changes;
    
    switch(lang) {
    case 1:
        cout << "日本語ロムが検出されましたが、現在サポートされていません。実装をリクエストするには、GitHubで問題を作成してください。" << endl;
        changes.open(arm9);
        break;
    case 2: //English
        cout << "English rom detected" << endl;
        changes.open(arm9);
        break;
    case 3: //French
        cout << "ROM française détectée" << endl;
        changes.open(arm9.substr(0, arm9.length() - 4) + "_french.csv");
        break;
    case 4: //Italian
        cout << "ROM italiana rilevata ma attualmente non supportata. Si prega di fare un problema su github per richiedere l'implementazione" << endl;
        changes.open(arm9);
        break;
    case 5: //German
        cout << "Deutsche ROM erkannt" << endl;
        changes.open(arm9.substr(0, arm9.length() - 4) + "_german.csv");
        break;
    case 164: //Spanish, this ROM is whacky and has random weird offsets so this is an ñ character representing something else
        cout << "ROM española detectada" << endl;
        changes.open(arm9.substr(0, arm9.length() - 4) + "_spanish.csv");
        break;
    default:
        cout << "WARNING: Detected language not recognized, there may be an error with your rom" << endl;
        cout << "Detected language ID: " << int(lang) << " (range is 1-7)" << endl;
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
