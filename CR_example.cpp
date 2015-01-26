#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include "CR.hpp"
#include <boost/algorithm/string.hpp>

using namespace std;

void test1(string genome_filename) {
    CR rm = CR(genome_filename, 100, true);

    string ll;
    while(cin) {
        cout << "waiting for input: " << endl;
        getline(cin, ll);
        //rm.find_reads(ll);
        rm.find_indexes(ll);
    }
}

void test2(string genome_filename, string superstring_filename,
        string positions_filename) {
    string superstring;
    vector<pair<int, int>> positions = vector<pair<int, int>>();
    tie(superstring, positions) = CR::preprocess(genome_filename, true);
    ofstream f1(superstring_filename);
    f1 << superstring;
    f1.close();

    ofstream f2(positions_filename);
    for (auto i : positions) {
        f2 << i.first << endl << i.second << endl;
    }
    f2.close();
}

void test3(string superstring_filename, string positions_filename) {
    string superstring;
    vector<pair<int, int>> positions = vector<pair<int, int>>();
    string l1;
    string l2;

    ifstream f1(superstring_filename);
    while (getline(f1, l1)) {
        boost::algorithm::trim(l1);
        superstring += l1;
    }

    ifstream f2(positions_filename);
    while (getline(f2, l1)) {
        getline(f2, l2);
        boost::algorithm::trim(l1);
        boost::algorithm::trim(l2);
        positions.push_back(pair<int, int>(stoi(l1), stoi(l2)));
    }

    CR rm = CR(superstring, positions, 100, true);

    string ll;
    while(cin) {
        cout << "waiting for input: " << endl;
        getline(cin, ll);
        rm.find_reads(ll);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " filename [filename] [filename]" << endl;
        cerr << "Examples: " << endl;
        cerr << "  Read .fastq file and construct CR-index" << endl;
        cerr << "  " << argv[0] << " bacteria.fastq" << endl;
        cerr << "  Read .fastq file, preprocess it and save metadata to provides files" << endl;
        cerr << "  " << argv[0] << " bacteria.fastq superstring.data positions.data" << endl;
        cerr << "  Read metadata and construct CR-index" << endl;
        cerr << "  " << argv[0] << " superstring.data positions.data" << endl;
        exit(1);
    }

    try {
        if (argc == 2) {
            test1(argv[1]);
        } else if (argc == 3) {
            test3(argv[1], argv[2]);
        } else if (argc == 4) {
            test2(argv[1], argv[2], argv[3]);
        }
    } catch(exception &e) {
        cerr << "Error: " << e.what() << endl;
        exit(1);
    }

    return 0;
}
