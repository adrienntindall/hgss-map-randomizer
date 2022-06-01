#ifndef HGSS_RANDO_H
#define HGSS_RANDO_H

#include <vector>
#include <string>

using std::vector;
using std::string;

typedef struct warp WARP;
using BLOCK = vector<WARP*>;
typedef struct Connection CONNECTION;

typedef struct warp {
    BLOCK* block = nullptr;
    string otherID;
    WARP* original; 
    WARP* newWarp;
    string warpID;
    string warpTo;
    short anchor;
    short binaryID;
    vector<CONNECTION> connections;
} WARP;

typedef struct Connection {
    WARP* warp;
    vector<string> locks;
} CONNECTION;

void GetWarpList();
void GetWarpDict();
void GetBlocks(const string& blockPath);
WARP* GetWarpByID(const string& ID);
bool RandomizeMap();
WARP* GetWarpByBinIDAndAnchor(short name, short anchor);
void SetWarps();
int getIndex(vector<WARP*> v, WARP* K);
int getIndex(vector<BLOCK*> v, BLOCK* K);
int getIndex(vector<string> v, const string& K);
bool CheckPath(WARP* from, WARP* to, const vector<string>& flags);
bool CheckPath(WARP* from, WARP* to, BLOCK* checked, const vector<string>& flags);
bool CheckFlags(const vector<string>& flags, vector<string> target);
void ClearData();
void GenerateLogFile(const string& path);

#endif
