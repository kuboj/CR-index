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

bool print(vector<int> v) {
    for (int i : v) {
        cout << i << ", ";
    }
    cout << endl;
    return true;
}

bool print(vector<string> v) {
    for (string i : v) {
        cout << i << endl;
    }
    return true;
}

bool query_ok(CRIndex *cr, HashIndex *hi, const string& query) {
    cout << "testing query: " << query << endl;
    chrono::time_point<std::chrono::system_clock> t1, t2;
    chrono::duration<double> elapsed;

    t1 = chrono::system_clock::now();
    vector<int> cr_indexes = cr->find_indexes(query);
    t2 = std::chrono::system_clock::now();
    elapsed = t2 - t1;
    cout << "cr->find_indexes took " << elapsed.count() << endl;
    dprint(cr_indexes);

    t1 = chrono::system_clock::now();
    vector<int> hi_indexes = hi->find_indexes(query);
    t2 = std::chrono::system_clock::now();
    elapsed = t2 - t1;
    cout << "hi->find_indexes took " << elapsed.count() << endl;
    dprint(hi_indexes);
//
//    t1 = chrono::system_clock::now();
//    vector<string> cr_reads = cr->find_reads(query);
//    t2 = std::chrono::system_clock::now();
//    elapsed = t2 - t1;
//    cout << "cr->find_reads took " << elapsed.count() << endl;
//    dprint(cr_reads);
//
//    t1 = chrono::system_clock::now();
//    vector<string> hi_reads = hi->find_reads(query);
//    t2 = std::chrono::system_clock::now();
//    elapsed = t2 - t1;
//    cout << "hi->find_reads took " << elapsed.count() << endl;
//    dprint(hi_reads);
//
//    cout << "occurrences: " << cr_indexes.size() << endl;

    return (cr_indexes.size() == hi_indexes.size() &&
//            cr_reads.size() == hi_reads.size() &&
            equal(cr_indexes.begin(), cr_indexes.end(), hi_indexes.begin()));// &&
//            equal(cr_reads.begin(), cr_reads.end(), hi_reads.begin()));
}

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
//    vector<string> retval = { "GATGTGTAAATCA" };
//    return retval;
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
    int read_length = 100;
    int query_length = 13;
    int num_of_queries = 100000;

    HashIndex hi = HashIndex(reads_filename, query_length, true);
    CRIndex cr = CRIndex(reads_filename, read_length, false);

    cout << fixed << "Testing " << num_of_queries << " queries of length " <<
            query_length << endl;
    vector<string> queries = generate_queries_from_file(num_of_queries,
            query_length, reads_filename);
    int c = 0;
    for (string q : queries) {
        c++;
        if (c % 100 == 0) {
            cout << c << "/" << num_of_queries << endl;
        }
        if (!query_ok(&cr, &hi, q)) {
            cerr << "query " << q << " failed." << endl;
            return false;
        }
    }

    return true;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Run CR-index against hashmap." << endl;
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
    } catch(exception &e) {
        cerr << "Error: " << e.what() << endl;
        exit(1);
    }
}
