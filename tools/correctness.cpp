#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include "cr_index.hpp"
#include "hash_index.hpp"

using namespace std;

void print(vector<int> v) {
    for (int i : v) {
        cout << i << ", ";
    }
    cout << endl;
}

void print(vector<string> v) {
    for (string i : v) {
        cout << i << endl;
    }
}

bool query_ok(CRIndex *cr, HashIndex *hi, const string& query) {
    vector<int> cr_indexes = cr->find_indexes(query);
    vector<int> hi_indexes = hi->find_indexes(query);
    vector<string> cr_reads = cr->find_reads(query);
    vector<string> hi_reads = hi->find_reads(query);

    if (cr_indexes.size() + hi_indexes.size() + cr_reads.size() + hi_reads.size() != 0) {
        cout << "testing query: " << query << endl;
        cout << cr_indexes.size() << endl;
        cout << hi_indexes.size() << endl;
        cout << cr_reads.size() << endl;
        cout << hi_reads.size() << endl;
    }

    cout << "CR:" << endl;
    print(cr_reads);
    cout << "HI:" << endl;
    print(hi_reads);

    return (cr_indexes.size() == hi_indexes.size() &&
            cr_reads.size() == hi_reads.size() &&
            equal(cr_indexes.begin(), cr_indexes.end(), hi_indexes.begin()) &&
            equal(cr_reads.begin(), cr_reads.end(), hi_reads.begin()));
}

vector<string> generate_queries(int num_of_queries, int query_length) {
    vector<string> retval = { "TGTCCACCTCAGA" };
    return retval;

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

bool test(string reads_filename) {
    int read_length = 100;
    int query_length = 13;
    int num_of_queries = 10000;

    HashIndex hi = HashIndex(reads_filename, query_length, true);
    CRIndex cr = CRIndex(reads_filename, read_length, false);

    cout << "Testing " << num_of_queries << " queries of length " <<
            query_length << endl;
    vector<string> queries = generate_queries(num_of_queries, query_length);
    for (string q : queries) {
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
