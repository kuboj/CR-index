#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "cr_index.hpp"
#include "hash_index.hpp"
#include <libGkArrays/gkArrays.h>

using namespace std;

bool test(string index_type, string reads_filename, string queries_filename,
        int read_length, int query_length, string query_type) {
    chrono::time_point<std::chrono::system_clock> t1, t2;
    chrono::duration<double> elapsed;

    cout << "index_type: " << index_type << endl;
    cout << "read_filename: " << reads_filename << endl;
    cout << "queries_filename: " << queries_filename << endl;
    cout << "read_length: " << read_length << endl;
    cout << "query_length: " << query_length << endl;
    cout << "query_type: " << query_type << endl;

    ifstream queries_s(queries_filename);
    vector<string> queries;
    string l;
    while (getline(queries_s, l)) {
        queries.push_back(l);
    }

    cout << "number of queries: " << queries.size() << endl;

    if (query_type != "index" && query_type != "read") {
        cerr << "Unknown query_type '" << query_type << "'" << endl;
        return false;
    }

    if (index_type != "cr" && index_type != "hash" && index_type != "gk") {
        cerr << "Unknown index_type '" << index_type << "'" << endl;
        return false;
    }

    cout << "Constructing index ... " << endl;
    if (index_type == "cr") {
        CRIndex cr = CRIndex(reads_filename, read_length, false);

        cout << "Querying ..." << endl;
        t1 = std::chrono::system_clock::now();
        for (string q : queries) {
            if (query_type == "index") {
                cr.find_indexes(q);
            } else if (query_type == "reads") {
                cr.find_reads(q);
            }
        }
    } else if (index_type == "hash") {
        HashIndex h = HashIndex(reads_filename, query_length,
                query_type == "read");

        cout << "Querying ..." << endl;
        t1 = std::chrono::system_clock::now();
        for (string q : queries) {
            if (query_type == "index") {
                h.find_indexes(q);
            } else if (query_type == "reads") {
                h.find_reads(q);
            }
        }
    } else if (index_type == "gk") {
        char* f = const_cast<char*>(reads_filename.c_str());
        gkarrays::gkArrays *gk = new gkarrays::gkArrays(f, query_length,
                true, 0, true, 4);

        cout << "Querying ..." << endl;
        t1 = std::chrono::system_clock::now();
        for (string q : queries) {
            if (query_type == "index") {
                uint num;
                auto *occurrences = gk->getTagsWithFactor(&q[0u],
                        static_cast<unsigned int>(q.size()), num);

                delete[] occurrences;
            } else if (query_type == "reads") {
                uint num;
                auto *occurrences = gk->getTagsWithFactor(&q[0u],
                        static_cast<unsigned int>(q.size()), num);
                for (uint i = 0; i < num; i++) {
                    gk->getTag(occurrences[i].first);
                }

                delete[] occurrences;
            }
        }
    }
    t2 = std::chrono::system_clock::now();
    elapsed = t2 - t1;
    cout << fixed << "Querying took " << elapsed.count() << "s" << endl;

    return true;
}

int main(int argc, char** argv) {
    if (argc < 7) {
        cerr << "Construct index " << endl;
        cerr << "Usage: " << argv[0] << " <index_type> <reads_filename> "  <<
                "<queries_filename> <read_length> <query_length> " <<
                "<query_type>" << endl;
        cerr << "index_type: cr, hash, gk" << endl;
        cerr << "query_type: index, read" << endl;
        cerr << "Example: " << endl;
        cerr << "  " << argv[0] << "cr bacteria.fastq queries.data " <<
                "101 13 index" << endl;
        exit(1);
    }

    try {
        if (test(argv[1], argv[2], argv[3], stoi(argv[4]), stoi(argv[5]),
                argv[6])) {
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
