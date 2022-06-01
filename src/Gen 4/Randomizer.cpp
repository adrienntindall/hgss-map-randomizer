#include "Randomizer.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>

#include "RomHandler.h"

using namespace std;
using std::filesystem::directory_iterator;
using std::filesystem::recursive_directory_iterator;

#define WARP_DATA "./files/Warps.csv"
#define WARP_DICT  "./files/WarpDictionary.csv"
#define FILE_PATH  "./temp/event_data/"

static vector<WARP*> needsConnections;
static vector<WARP> sWarpList;
static vector<BLOCK> sBlocks;
static BLOCK sGarbageRooms;
static BLOCK sRequiredDeadEnds;
static BLOCK sUnusedWarps;
static BLOCK sUsedWarps;
static vector<BLOCK*> sUnusedBlocks;
static WARP* startingPoint;
typedef struct progression {
    WARP* warp;
    vector<string> flags;
} PROGRESSION;
static vector<PROGRESSION> sProgression;
typedef struct FlagAlias {
    string flag;
    vector<string> def;
    vector<string> equiv;
    vector<WARP*> checks;
    bool set = false;
} FLAG_ALIAS;
static vector<FLAG_ALIAS> sOtherFlags;
static vector<string> sMainFlags;
typedef struct Firsts {
    BLOCK* block;
    WARP* first;
} FIRST;
static vector<FIRST> sFirstList;
static WARP* GetFirst(BLOCK* block);

void AddConnection(WARP* target, WARP* connection, vector<string> flagList) {
    CONNECTION con;
    con.warp = connection;
    con.locks = std::move(flagList);
    target->connections.push_back(con);
}

void ReadSpecialBlockInput(WARP* pivot, vector<string> lines, int start, int dir) {
    vector<string> flagList;
    for(int c = start + dir; (dir == 1) ? (c < lines.size()) : (c >= 0); c += dir) {
        WARP* ptr = GetWarpByID(lines[c]);
        if(ptr != nullptr) {
            AddConnection(pivot, ptr, flagList);
        }
        else if(lines[c].find("FLAG_") != string::npos) {
            flagList.push_back(lines[c]);
        }
        else if(dir == 1) {
            if(lines[c].find("LEDGE_UP") != string::npos) {
                return;
            }
        }
        else if(dir == -1) {
            if(lines[c].find("LEDGE_DOWN") != string::npos) {
                return;
            }
        }
    }
}

void GetBlocks(const string& blockPath) {
    string line;
    cout << "Getting Blocks!" << endl;
    for(const auto & file : recursive_directory_iterator(blockPath)) {
        string path = file.path().string();
        //cout << path << endl;
        ifstream block;
        block.open(file.path());
        string comp = path.substr(path.find_last_of('.') + 1, path.length() - 1);         
        if(comp == "blk") { //BLocKs
            auto* newBlock = new BLOCK;
            vector<string> lines;
            while(getline(block, line)) {
                lines.push_back(line);  
            }
            for(int c = 0; c < lines.size(); c++) {
                WARP* ptr = GetWarpByID(lines[c]);
                if(ptr == nullptr) continue;
                ReadSpecialBlockInput(ptr, lines, c, 1);
                ReadSpecialBlockInput(ptr, lines, c, -1);
                newBlock->push_back(ptr);
                ptr->block = newBlock;
                sUnusedWarps.push_back(ptr);
            }
            if(path.find("NewBark") != string::npos) {
                startingPoint = newBlock->at(0);
                for(auto & c : *newBlock) {
                    needsConnections.push_back(c);
                }
            }
            else {
                sUnusedBlocks.push_back(newBlock);
            }
        }
        else if(comp == "grn") { //GaRbage dead eNds
            while(getline(block, line)) {
                WARP* ptr = GetWarpByID(line);
                if(ptr != nullptr) {
                    ptr->block = nullptr;
                    sGarbageRooms.push_back(ptr);
                    sUnusedWarps.push_back(ptr);
                }
            }
        }
        else if(comp == "red") { //REquired Dead ends
            while(getline(block, line)) {
                WARP* ptr = GetWarpByID(line);
                if(ptr != nullptr) {
                    ptr->block = nullptr;
                    sRequiredDeadEnds.push_back(ptr);
                    sUnusedWarps.push_back(ptr);
                }
            }
        }
        else if(comp == "pnk") { //Progression aNd checK flags (I tried)
            vector<string> flaglist;
            while(getline(block, line)) {
                if(line.find("FLAG_") != string::npos) {
                    flaglist.push_back(line);
                    sMainFlags.push_back(line);
                }
                else {
                    WARP* ptr = GetWarpByID(line);
                    PROGRESSION pog = {
                        ptr,
                        flaglist
                    };
                    sProgression.push_back(pog);
                }
            }
        }
        else if(comp == "blu") { //yeah I have no good one for this it's just like flag requirements that are way more up in the air
            FLAG_ALIAS fa;
            getline(block, line);
            fa.flag = line;
            while(getline(block, line)) {
                if(line.find("FLAG_") != string::npos) {
                    fa.def.push_back(line);
                }
                else {
                    WARP* ptr = GetWarpByID(line);
                    if(ptr != nullptr) fa.checks.push_back(ptr);
                }
            }
            sOtherFlags.push_back(fa);
        }
        block.close();
    }
    cout << "Blocks gotten!" << endl;
}

WARP* GetWarpByID(const string& ID) {
    for(auto & c : sWarpList) {
        if(c.warpID == ID) {
            return &c;
        }
    }
    if((ID.find("FLAG_") == string::npos) && (ID.find("LEDGE_") == string::npos)) {
        cout << "Warning: " << ID << " doesn't have a valid original" << endl;
    }
    return nullptr;
}

inline string csvGotoNext(const string& str) {
    return str.substr(str.find(',') + 1, str.length());
}

inline string csvGetNext(const string& str) {
    return str.substr(0, str.find(','));
}

void GetWarpList() {
    cout << "Reading in warps!" << endl;
    string line;
    ifstream warpData;
    warpData.open(WARP_DATA);
    while(getline(warpData, line)) {
        WARP newWarp;
        newWarp.warpID = csvGetNext(line);
        if(newWarp.warpID == "STOP") break;
        //cout << newWarp.warpID << endl;
        line = csvGotoNext(line);
        newWarp.warpTo = csvGetNext(line);
        line = csvGotoNext(line);
        newWarp.anchor = stoi(csvGetNext(line));
        line = csvGotoNext(line);
        newWarp.otherID = csvGetNext(line);
        sWarpList.push_back(newWarp);
    }
    for(auto & c : sWarpList) {
        c.original = GetWarpByID(c.otherID);
        if(c.original == nullptr) cout << "WARNING: " << c.warpID << " is missing a definition for " << c.otherID << endl;
        c.newWarp = nullptr;
    }
    warpData.close();
    cout << "Done reading!" << endl;
}

void GetWarpDict() {
    string line;
    ifstream warpDict;
    warpDict.open(WARP_DICT);
    getline(warpDict, line); //burns the first line
    while (getline(warpDict, line)) {
        string sid = csvGetNext(line);
        line = csvGotoNext(line);
        short bid = stoi(line);
        //cout << sid << " " << bid << endl;
        for (auto & c : sWarpList) {
            if (sid == c.warpTo) {
                c.binaryID = bid;
            }
        }
    }
    warpDict.close();
}

WARP* GetWarpByBinIDAndAnchor(short name, short anchor) {
    for(auto & sUsedWarp : sUsedWarps) {
        if((name == sUsedWarp->binaryID) && (anchor == sUsedWarp->anchor)) return sUsedWarp;
    }
    return nullptr;
}

void ConnectWarps(WARP* A, WARP* B) {
    if((A == nullptr) || (B == nullptr)) {
        cout << "Warning: trying to connect nullptr warp" << endl;
        return;
    }
    if(B->original == nullptr) {
        cout << "Warning: " << B->warpID << " has a nullptr original" << endl;
        return;
    }
    if(A->original == nullptr) {
        cout << "Warning: " << A->warpID << " has a nullptr original" << endl;
        return;
    }
    A->newWarp = B->original;
    B->newWarp = A->original;
    //cout << "Connected: " << A->warpID << " and " << B->warpID << endl;
}

void SwapConnections(WARP* A, WARP* B) {
    if((A->newWarp == nullptr) && (B->newWarp != nullptr)) {
        ConnectWarps(A, B->newWarp->original);
        B->newWarp = nullptr;
        swap(sUsedWarps[getIndex(sUsedWarps, B)], sUsedWarps[sUsedWarps.size()-1]);
        sUsedWarps.pop_back();
        sUsedWarps.push_back(A);
        swap(sUnusedWarps[getIndex(sUnusedWarps, A)], sUnusedWarps[sUnusedWarps.size()-1]);
        sUnusedWarps.pop_back();
        sUnusedWarps.push_back(B);
    }
    else if ((A->newWarp != nullptr) && (B->newWarp == nullptr)) {
        SwapConnections(B, A);
    }
    else if((A->newWarp != nullptr) && (B->newWarp != nullptr)) {
        WARP* ptr = A->newWarp->original;
        ConnectWarps(A, B->newWarp->original);
        ConnectWarps(B, ptr);
    }
    else {
        cout << "Waring: trying to swap two warps that haven't been given a new warp" << endl;
    }
}

void InsertWarps(WARP* A, WARP* B, WARP* insertionPoint) {
    WARP* warp2 = insertionPoint->newWarp->original;
    ConnectWarps(A, insertionPoint);
    ConnectWarps(B, warp2);
}

inline int readWord(vector<unsigned char> binData, int c) {
    return binData[c] + (binData[c + 1] << 8) + (binData[c + 2] << 16) + (binData[c + 3] << 24);
}

inline short readShort(vector<unsigned char> binData, int c) {
    return binData[c] + (binData[c + 1] << 8);
}

void SetWarps() {
    string line;
    cout << "Setting Warps!" << endl;
    for(const auto & file : directory_iterator(FILE_PATH)) {
        //cout << file.path() << endl;
        
        fstream bin;
        bin.open(file.path(), ios::binary|ios::in|ios::out|ios::ate);
        
        bin.seekg(0, ios::end);
        
        streampos size = bin.tellg();
        
        bin.seekp(0);
        
        vector<unsigned char> binData(size);
        bin.read((char*) &binData[0], size);
        
        int pos = 0;
        
        //Skipping bgs structs (size: 5 words = 0x14 bytes)
        pos += 4 + readWord(binData, pos) * 0x14;
        
        //Skipping obj structs (size: 8 words = 0x20 bytes)
        pos += 4 + readWord(binData, pos) * 0x20;
        
        //Reading in number of warps
        int warpAmount = readWord(binData, pos);
        //cout << pos << " " << warpAmount << endl;
        pos += 4;
        
        for(int w = 0; w < warpAmount; w++) {
            //we don't need the first word (position value)
            pos += 4;
            short mapHeaderID = readShort(binData, pos);
            short anchor = readShort(binData, pos + 2);
            //cout << "BinID: " << mapHeaderID << " Anchor: " << anchor << endl;
            WARP* warp = GetWarpByBinIDAndAnchor(mapHeaderID, anchor);
            if (warp != nullptr) {
                //cout << "Changing " << warp->warpID << " to " << warp->newWarp->warpID << endl;
                bin.seekp(pos);
                bin.write((char*) &warp->newWarp->binaryID, 2);
                bin.seekp(pos + 2);
                bin.write((char*) &warp->newWarp->anchor, 2);
            }
            pos += 8; //this skips us ahead past the height and to the beginning of the next warp =)
        }
        
        bin.close();
    }
    cout << "Warps Set!" << endl;
}

bool RandomizeMap() {
    cout << "Starting Randomization!" << endl;
    auto rd = random_device {}; 
    auto rng = default_random_engine { rd() };
    shuffle(begin(sUnusedWarps), end(sUnusedWarps), rng);
    
    while(!needsConnections.empty()) {
        shuffle(begin(needsConnections), end(needsConnections), rng);

        WARP* warp = sUnusedWarps.back();
        WARP* neededWarp = needsConnections.back();
        ConnectWarps(warp, neededWarp);
        
        sUsedWarps.push_back(warp);
        sUsedWarps.push_back(neededWarp);
        
        sUnusedWarps.pop_back();
        needsConnections.pop_back();
        
        int index = getIndex(needsConnections, warp);
        if(index != -1) {
            swap(needsConnections[index], needsConnections[needsConnections.size()-1]);
            needsConnections.pop_back();
        }
        index = getIndex(sUnusedWarps, neededWarp);
        if(index != -1) {
            swap(sUnusedWarps[index], sUnusedWarps[sUnusedWarps.size()-1]);
            sUnusedWarps.pop_back();
        }
        index = getIndex(sUnusedBlocks, warp->block);
        if((warp->block != nullptr) && (index != -1)) {
            BLOCK* blk = warp->block;
            for(int c = 0; c < blk->size(); c++) {
                if(find(begin(sUnusedWarps), end(sUnusedWarps), (*blk)[c]) != sUnusedWarps.end()) {
                    needsConnections.push_back((*blk)[c]);
                }
                else {
                    FIRST f = { blk, (*blk)[c] };
                    sFirstList.push_back(f);
                }
            }
            swap(sUnusedBlocks[index], sUnusedBlocks[sUnusedBlocks.size()-1]);
            sUnusedBlocks.pop_back();
        }
        
        while(needsConnections.empty() && !sUnusedBlocks.empty()) {
            shuffle(begin(sUnusedBlocks), end(sUnusedBlocks), rng);
            shuffle(begin(sUsedWarps), end(sUsedWarps), rng);
            if(sUsedWarps.back() == sUsedWarps.back()->newWarp) continue;
            BLOCK* blk = sUnusedBlocks.back();
            shuffle(begin(*blk), end(*blk), rng);
            InsertWarps((*blk)[0], (*blk)[1], sUsedWarps.back()); //A <-> sUsedWarps.back, B <-> sUsedWarps.back.newWarp
            if(sUsedWarps.back()->block == nullptr) { //if A got connected to a dead end, then B is the nearest connection to the center
                FIRST f = { blk, (*blk)[1] };
                sFirstList.push_back(f);
            }
            else if(sUsedWarps.back() == GetFirst(sUsedWarps.back()->block)){ //if A got connected to a nearest element, then B is the new nearest element
                FIRST f = { blk, (*blk)[1] };
                sFirstList.push_back(f);
            }
            else { //if A did NOT get connected to a nearest element/dead end, then it must be the nearest element
                FIRST f = { blk, (*blk)[0] };
                sFirstList.push_back(f);
            }
            sUsedWarps.push_back((*blk)[0]);
            sUsedWarps.push_back((*blk)[1]);
            for(int c = 2; c < blk->size(); c++) {
                needsConnections.push_back((*blk)[c]);
            }
            sUnusedBlocks.pop_back();
        }    
    }
    //Red Rooms replacing green rooms
    for (auto & sRequiredDeadEnd : sRequiredDeadEnds) {
        if(getIndex(sUnusedWarps, sRequiredDeadEnd) == -1) continue;
        shuffle(begin(sGarbageRooms), end(sGarbageRooms), rng);
        for (auto & sGarbageRoom : sGarbageRooms) {
            int g_index = getIndex(sUsedWarps, sGarbageRoom);
            if(g_index == -1) continue;
            SwapConnections(sRequiredDeadEnd, sGarbageRoom);
            break;
        }
    }
    //Blu flag handling
    for(int t = 0; t < 15; t++) { //do this 5 times to achieve a theoretical minimum; the more the slower but better
        shuffle(begin(sOtherFlags), end(sOtherFlags), rng);
        for (auto & sOtherFlag : sOtherFlags) {
            sOtherFlag.set = false;
            sOtherFlag.equiv.clear();
            sOtherFlag.equiv.insert(sOtherFlag.equiv.end(), sOtherFlag.def.begin(), sOtherFlag.def.end());
            vector<string> required;
            for(int f = 0; f < sMainFlags.size() + 1; f++) {
                bool check = true;
                for(int w = 0; w < sOtherFlag.checks.size(); w++) {
                    check = CheckPath(startingPoint, sOtherFlag.checks[w], sOtherFlag.equiv);
                    if(!check) {
                       break;
                    }
                }
                if(check) break;
                if(f == sMainFlags.size()) continue;
                sOtherFlag.equiv.push_back(sMainFlags[f]);
                if(f == sMainFlags.size() + 1) {
                    sOtherFlag.equiv.emplace_back("FLAG_IMPOSSIBLE");
                }
            }
            sOtherFlag.set = true;
        }
    }
    //Main Progression check
    while(!sProgression.empty()) {
        shuffle(begin(sGarbageRooms), end(sGarbageRooms), rng);
        if(!CheckPath(startingPoint, sProgression.back().warp, sProgression.back().flags)) {
           int c;
           for(c = 0; c < sGarbageRooms.size(); c++) {
               if(getIndex(sUsedWarps, sGarbageRooms[c]) != -1) {
                    if (sProgression.back().warp->block == nullptr) { //Gyms, other rooms with one warp
                        SwapConnections(sGarbageRooms[c], sProgression.back().warp);
                    } 
                    else { //E4, other rooms with 2+ warps
                        SwapConnections(sGarbageRooms[c], GetFirst(sProgression.back().warp->block));
                    }
                    if(CheckPath(startingPoint, sProgression.back().warp, sProgression.back().flags)) {
                       break;
                    }
                    //Path unreachable, swap warps back to original positions
                    if (sProgression.back().warp->block == nullptr) {
                        SwapConnections(sGarbageRooms[c], sProgression.back().warp);
                    } 
                    else {
                        SwapConnections(sGarbageRooms[c], GetFirst(sProgression.back().warp->block));
                    }   
               }
           }
           if(c == sGarbageRooms.size()) {
               return false;
           }
        }
        sProgression.pop_back();
    }
    cout << "Randomized!" << endl;
    return true;
}

int getIndex(vector<WARP*> v, WARP* K) {
    auto it = find(v.begin(), v.end(), K);
    if (it != v.end())
    {
        int index = it - v.begin();
        return index;
    }
    else {
        return -1;
    }
}

int getIndex(vector<BLOCK*> v, BLOCK* K) {
    auto it = find(v.begin(), v.end(), K);
    if (it != v.end())
    {
        int index = it - v.begin();
        return index;
    }
    else {
        return -1;
    }
}

int getIndex(vector<string> v, const string& K) {
    auto it = find(v.begin(), v.end(), K);
    if (it != v.end())
    {
        int index = it - v.begin();
        return index;
    }
    else {
        return -1;
    }
}

bool CheckPath(WARP* from, WARP* to, const vector<string>& flags) {
    auto* checked = new BLOCK;
    bool ret = CheckPath(from, to, checked, flags) || CheckPath(from->newWarp->original, to, checked, flags);
    delete checked;
    return ret;
}

bool CheckPath(WARP* from, WARP* to, BLOCK* checked, const vector<string>& flags) {
    if(from == nullptr) return false;
    if(from == to) return true;
    checked->push_back(from);
    for(int c = 0; c < from->connections.size(); c++) {
        if(getIndex(*checked,from->connections[c].warp) != -1) continue;
        if(getIndex(*checked, to) != -1) {
            return true;
        }
        if(!CheckFlags(flags, from->connections[c].locks)) continue;
        if(CheckPath(from->connections[c].warp, to, checked, flags)) {
            return true;
        }
        if(CheckPath(from->connections[c].warp->newWarp->original, to, checked, flags)) {
            return true;
        }
    }
    return false;
}

int OtherFlagIndex(const string& flag) {
    for(int c = 0; c < sOtherFlags.size(); c++) {
        if(sOtherFlags[c].flag == flag) return c;
    }
    return -1;
}

bool CheckFlags(const vector<string>& flags, vector<string> target) {
    for(int c = 0; c < target.size(); c++) {
        int ofID = OtherFlagIndex(target[c]);
        if(ofID != -1) {
            if(!sOtherFlags[ofID].set) return false;
            target.erase(target.begin() + c);
            c--;
            for(auto & f : sOtherFlags[ofID].equiv) {
                target.push_back(f);
            }
            continue;
        }
        if(getIndex(flags, target[c]) == -1) return false;
    }
    return true;
}

void ClearData() {
    sWarpList.clear();
    needsConnections.clear();
    sBlocks.clear();
    sGarbageRooms.clear();
    sRequiredDeadEnds.clear();
    sUnusedWarps.clear();
    sUsedWarps.clear();
    sUnusedBlocks.clear();
    sProgression.clear();
    sOtherFlags.clear();
    sMainFlags.clear();
}

void GenerateLogFile(const string& path) {
    ofstream log;
    log.open(path);
    for (auto & c : sWarpList) {
        if(c.newWarp != nullptr) {
            log << c.warpID;
            log << " <-> ";
            log << c.newWarp->original->warpID;
            log << endl;
        }
    }
    log.close();
}

static WARP* GetFirst(BLOCK* block) {
    if(block == nullptr) cout << "WARNING: Trying to get the first of a nullptr" << endl;
    for(auto & c : sFirstList) {
        if(block == c.block) return c.first;
    }
    return (*block)[0];
}