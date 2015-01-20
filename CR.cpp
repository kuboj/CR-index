#include "CR.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include <stdexcept>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "FM.h"

using namespace std;
using namespace boost::filesystem;

const bool CR::DEFAULT_VERBOSITY = false;

CR::CR(string path, bool verbose) {
    this->verbose = verbose;
    this->reads = vector<string>();

    ifstream f(path.c_str());

    if (!f) {
        throw runtime_error("Error opening file '" + path + "'");
    }

    debug("File '" + path + "' opened, reading ...");

    string l;
    int i = 0;
    int skipped = 0;
    while (getline(f, l)) {
        if (i++ % 4 != 1) {
            continue;
        }
        boost::algorithm::trim(l);
        if (check_read(l)) {
            this->reads.push_back(l);
        } else {
            skipped++;
        }
    }
    debug("Loaded " + to_string(this->reads.size()) + " reads. Skipped " +
            to_string(skipped));

    path tmpdir = temp_directory_path();

//            uint8_t* buffer = new uint8_t[2000000000];
//            uint32_t n = 0;
//
//            string l;
//            while (getline(f, l)) {
//                for (int i = 0; i < l.length(); i++) {
//                    buffer[n] = l[i];
//                    n++;
//                }
//            }
//            f.close();
//            l.clear();
//
//            uint8_t* T = new uint8_t[n + 1];
//            for (int i = 0; i < n; i++) {
//                T[i] = buffer[i];
//            }
//            delete buffer;
//
//            T[n] = '\0';
//            cout << "Starting building FM index..." << endl;
//            FM* q = new FM(T, n, 64);
//
//            if (!q) {
//                cerr << "FM index building failed. fuck you. that's why." << endl;
//                exit(1);
//            }
//
//            cout << "FM index building done" << endl;
//
//            // tests
//
//            uint8_t* s = (uint8_t*)"AAAAAAAAAA";
//            int sl = 10;
//            uint32_t cnt;
//            cnt = q->count(s, sl);
//            cout << "count of  '" << s << "' = " << cnt << endl;
//
//            uint32_t matches;
//            uint32_t* locs;
//            locs = q->locate(s, sl, &matches);
//            cout << "locations of '" << s << "' :";
//            for (int i = 0; i < matches; i++) {
//                cout << locs[i] << ", ";
//            }
//            cout << endl;
//
//            while(true) { }
//            delete q;
}

void CR::debug(string msg) {
    if (this->verbose) {
        cerr << msg << endl;
    }
    return;
}

// TODO: modify this when taking into account reads with uNknowns etc. (R, ... )
bool CR::check_read(string r) {
    for (int i = 0; i < r.size(); i++) {
        if ((r[i] != 'A') &&
                (r[i] != 'C') &&
                (r[i] != 'T') &&
                (r[i] != 'G')) {
            return false;
        }
    }

    return true;
}

CR::~CR() {
    //
}
