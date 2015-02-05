#ifdef DEBUG
#define dprint print
#else
#define dprint 0 && print
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <chrono>
#include "cr_index.hpp"
#include "hash_index.hpp"

using namespace std;

vector<string> generate_random_queries(int num_of_queries, int query_length) {
    vector<string> retval;

    srand(time(NULL));
    string chars = "ACTG";
    for (int i = 0; i < num_of_queries; i++) {
        string q = "";
        for (int j = 0; j < query_length; j++) {
            q += chars[rand() % 4];
        }
        retval.push_back(q);
    }

    return retval;
}

vector<string> generate_queries_from_file(int num_of_queries, int query_length,
        string reads_filename) {
    vector<string> retval;

    ifstream f(reads_filename);
    string l;
    string read;
    srand(time(NULL));
    getline(f, l);
    getline(f, read);
    getline(f, l);
    getline(f, l);
    while (retval.size() < (unsigned)num_of_queries) {
       if (rand() % 2 == 0) {
           if (f.eof()) {
               f.clear();
               f.seekg(0);
           }
           getline(f, l);
           getline(f, read);
           getline(f, l);
           getline(f, l);
       }
       retval.push_back(read.substr(rand() % (read.length() - query_length - 1),
               query_length));
    }
    f.close();

    return retval;
}

bool test(string reads_filename) {
    ifstream f(reads_filename);
    string l;
    getline(f, l);
    getline(f, l);
    int read_length = l.size();
    int query_length = 13;
    int num_of_queries = 10000;

    CRIndex cr = CRIndex(reads_filename, read_length, false);

    cout << fixed << "Testing " << num_of_queries << " queries of length " <<
            query_length << " file: " << reads_filename << endl;
    vector<string> queries = generate_queries_from_file(num_of_queries,
            query_length, reads_filename);
    int c = 0;

    chrono::time_point<std::chrono::system_clock> t1, t2;
    chrono::duration<double> elapsed;
    t1 = std::chrono::system_clock::now();

    for (string q : queries) {
        c++;
        if (c % 100 == 0) {
            cout << c << "/" << num_of_queries << endl;
        }
        vector<int> cr_indexes = cr.find_indexes(q);
    }

    t2 = std::chrono::system_clock::now();
    elapsed = t2 - t1;
    cout << num_of_queries << " took " << elapsed.count() << "s" << endl;

    return true;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Run CR-index " << endl;
        cerr << "Usage: " << argv[0] << " filename" << endl;
        cerr << "Example: " << endl;
        cerr << "  " << argv[0] << " bacteria.fastq" << endl;
        exit(1);
    }

    try {
        if (test(argv[1])) {
            cout << "OK." << endl;
            return 0;
        } else {
            cerr << "Failed to pass." << endl;
            exit(1);
        }
    } catch (exception &e) {
        cerr << "Error: " << e.what() << endl;
        exit(1);
    }
}
