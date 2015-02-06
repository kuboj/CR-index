#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include "cr_index.hpp"

using namespace std;

void test1(string genome_filename) {
    ifstream f(genome_filename);
    string l;
    getline(f, l);
    getline(f, l);
    int read_length = l.size();

    cout << "Building CRIndex ... " << endl;
    CRIndex rm = CRIndex(genome_filename, read_length, true);

    string ll;
//    vector<string> queries = {"TTTAAAGCTTCAG", "TAATGTCTGGAAT", "TAATTTTTTTATA",
//                "GTTTTTGGTGAAA", "GTAATGTTGTTTT", "ATATCGACGTCTT", "AAAAAAAAAAAAA"};
    while(cin) {
//    for (string ll : queries) {
        cout << "waiting for input: " << endl;
        getline(cin, ll);
        boost::algorithm::trim_copy(ll);
        chrono::time_point<std::chrono::system_clock> t1, t2;
        chrono::duration<double> elapsed;

        t1 = chrono::system_clock::now();
        vector<string> r1 = rm.find_reads(ll);
        cout << r1.size() << " occurences" << endl;
//        for (string s : r1) {
//            cout << s << endl;
//        }

        t2 = std::chrono::system_clock::now();

        elapsed = t2 - t1;
        cout << "cr->find_reads(" << ll << ")  took " << elapsed.count() << endl << endl;
//        vector<int> r2 = rm.find_indexes(ll);
//        for (int i: r2) {
//            cout << i << endl;
//        }
    }
}

//void test2(string genome_filename, string superstring_filename,
//        string positions_filename) {
//    string superstring;
//    vector<pair<int, int>> positions = vector<pair<int, int>>();
//    vector<tuple<int, int, char>> diff = vector<tuple<int, int, char>>();
//    tie(superstring, positions, diff) = CR::preprocess(genome_filename, true);
//    ofstream f1(superstring_filename);
//    f1 << superstring;
//    f1.close();
//
//    ofstream f2(positions_filename);
//    for (auto i : positions) {
//        f2 << i.first << endl << i.second << endl;
//    }
//    f2.close();
//}

//void test3(string superstring_filename, string positions_filename) {
//    string superstring;
//    vector<pair<int, int>> positions = vector<pair<int, int>>();
//    string l1;
//    string l2;
//
//    ifstream f1(superstring_filename);
//    while (getline(f1, l1)) {
//        boost::algorithm::trim(l1);
//        superstring += l1;
//    }
//
//    ifstream f2(positions_filename);
//    while (getline(f2, l1)) {
//        getline(f2, l2);
//        boost::algorithm::trim(l1);
//        boost::algorithm::trim(l2);
//        positions.push_back(pair<int, int>(stoi(l1), stoi(l2)));
//    }
//
//    CR rm = CR(superstring, positions, 100, true);
//
//    string ll;
//    while(cin) {
//        cout << "waiting for input: " << endl;
//        getline(cin, ll);
//        rm.find_reads(ll);
//    }
//}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <filename> [filename] [filename]" << endl;
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
            //test3(argv[1], argv[2]);
        } else if (argc == 4) {
            //test2(argv[1], argv[2], argv[3]);
        }
    } catch(exception &e) {
        cerr << "Error: " << e.what() << endl;
        exit(1);
    }

    return 0;
}
