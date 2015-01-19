#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
#include "FM.h"

using namespace std;

int main(int argc,char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " filename" << endl;
        exit(1);
    }
  
    string filename = argv[1];
    ifstream f(filename.c_str());
    if (!f) {
        cerr << "Error opening file " << filename << endl;
        exit(1);
    }

    uint8_t* buffer = new uint8_t[2000000000];
    uint32_t n = 0;

    string l;
    while (getline(f, l)) {
        for (int i = 0; i < l.length(); i++) {
            buffer[n] = l[i];
            n++;
        }
    }
    f.close();
    l.clear();

    uint8_t* T = new uint8_t[n + 1];
    for (int i = 0; i < n; i++) {
        T[i] = buffer[i];
    }
    delete buffer;

    T[n] = '\0';
    cout << "Starting building FM index..." << endl;
    FM* q = new FM(T, n, 64);

    if (!q) {
        cerr << "FM index building failed. fuck you. that's why." << endl;
        exit(1);
    }

    cout << "FM index building done" << endl;

    // tests

    uint8_t* s = (uint8_t*)"AAAAAAAAAA";
    int sl = 10;
    uint32_t cnt;
    cnt = q->count(s, sl);
    cout << "count of  '" << s << "' = " << cnt << endl;

    uint32_t matches;
    uint32_t* locs;
    locs = q->locate(s, sl, &matches);
    cout << "locations of '" << s << "' :";
    for (int i = 0; i < matches; i++) {
        cout << locs[i] << ", ";
    }
    cout << endl;

    while(true) { }
    delete q;
}
