#include "CR.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <string>
#include <exception>
#include <stdexcept>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <pstreams/pstream.h>
#include "FM.h"

using namespace std;

const bool CR::DEFAULT_VERBOSITY = false;

CR::CR(string path, bool verbose) {
    this->verbose = verbose;
    this->reads = vector<string>();
    this->positions = vector<pair<int, int>>();
    this->superstring = "";

    boost::filesystem::path genome_path = boost::filesystem::path(path);

    ifstream f(genome_path.string().c_str());

    if (!f) {
        throw runtime_error("Error opening file '" +
                genome_path.string()+ "'");
    }

    debug("File '" + genome_path.string() + "' opened, reading ...");

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

    boost::filesystem::path tmpdir = boost::filesystem::temp_directory_path();
    srand (time(NULL));
    tmpdir += "/" + to_string(rand() % 1000000) + "/";
    string prefix = to_string(rand() % 1000000);
    debug("creating temporary directory " + tmpdir.string() + "\n");
    if (!boost::filesystem::create_directory(tmpdir)) {
        throw runtime_error("failed to create temporary directory");
    }

    // sga index
    string cmd = "sga index -v -a ropebwt -c -t 4 -p " + tmpdir.string() +
            "/" + prefix + " " + genome_path.string();
    redi::ipstream ips(cmd);
    string s;
    while (getline(ips, s)) {
        info(s);
    }
    ips.close();
    int exit_code = ips.rdbuf()->status();
    if (exit_code != 0) {
        throw runtime_error("Error while executing '" + cmd +
                "'. Exit code: " + to_string(exit_code));
    }

    // sga overlap
    string cmd2 = "sga overlap -v -t 4 -p " + tmpdir.string() + "/" + prefix +
            " " + genome_path.string();
    redi::ipstream ips2(cmd2);
    string s2;
    while (getline(ips2, s2)) {
        info(s2);
    }
    ips2.close();
    int exit_code2 = ips2.rdbuf()->status();
    if (exit_code2 != 0) {
        throw runtime_error("Error while executing '" + cmd2 +
                "'. Exit code: " + to_string(exit_code));
    }

    boost::filesystem::path overlap_path = boost::filesystem::path(
            boost::filesystem::current_path() / (genome_path.stem().string() + ".asqg.gz"));
    if (!boost::filesystem::exists(overlap_path)) {
        throw runtime_error("Error. Overlap file '" + overlap_path.string() +
                "' not found");
    }

    // move overlap file to tmpdir
    info(overlap_path.string() + " created.");
    boost::filesystem::path aux = boost::filesystem::path(tmpdir / (prefix +
            ".asqg.gz"));
    boost::filesystem::rename(overlap_path, aux);
    overlap_path = aux;

    // sga assemble
    string cmd3 = "sga assemble -v -o " + tmpdir.string() +
            "/" + prefix + " " + overlap_path.string();
    redi::ipstream ips3(cmd3);
    string s3;
    while (getline(ips3, s3)) {
        info(s3);
    }
    ips3.close();
    int exit_code3 = ips3.rdbuf()->status();
    if (exit_code3 != 0) {
        throw runtime_error("Error while executing '" + cmd3 +
                "'. Exit code: " + to_string(exit_code3));
    }

    boost::filesystem::path contigs_path = boost::filesystem::path(
            tmpdir / (prefix + "-contigs.fa"));
    if (!boost::filesystem::exists(contigs_path)) {
        throw runtime_error("Error. Contigs file '" + overlap_path.string() +
                "' not found");
    }

    info(contigs_path.string() + " created.");

    // read contigs
    ifstream f2(contigs_path.string());

    if (!f2) {
        throw runtime_error("Error opening file '" +
                contigs_path.string() + "'");
    }

    l = "";
    i = 0;
    skipped = 0;
    while (getline(f2, l)) {
        if (i++ % 2 != 1) {
            continue;
        }
        boost::algorithm::trim(l);
        if (check_contig(l)) {
            this->superstring += l;
        } else {
            skipped++;
        }
    }

    // construct FM-index, find each read in it and add missing reads
    uint8_t* T = new uint8_t[this->superstring.length() + 1];
    uint32_t n = this->superstring.length();
    for (int i = 0; i < n; i++) {
        T[i] = this->superstring[i];
    }
    T[n] = '\0';

    debug("Starting building FM-index ...");
    FM* fm = new FM(T, n, 64);
    if (!fm) {
        throw runtime_error("FM index building failed. Fuck you. That's why.");
    }
    debug("FM index building done");

    int missing_read_count = 0;
    string missing_reads_string = "";

    debug("Querying FM index for reads");
    for (int i = 0; i < this->reads.size(); i++) {
        if (i % 10000 == 0) {
            debug("Processing reads " + to_string(i) + " of " +
                    to_string(this->reads.size()));
        }
        string read = this->reads[i];
        uint8_t* r = new uint8_t[read.length() + 1];
        for (int j = 0; j < read.length(); j++) {
            r[j] = read[j];
        }
        r[read.length()] = '\0';

        uint32_t num_matches;
        uint32_t* indexes;
        indexes = fm->locate(r, read.length(), &num_matches);

        /////
        string read2 = rev_compl(read);
        uint8_t* r2 = new uint8_t[read2.length() + 1];
        for (int j = 0; j < read2.length(); j++) {
            r2[j] = read2[j];
        }
        r2[read2.length()] = '\0';

        uint32_t num_matches2;
        uint32_t* indexes2;
        indexes2 = fm->locate(r2, read2.length(), &num_matches2);
        /////

        if (num_matches == 0 && num_matches2 == 0) {
            missing_read_count++;
            this->positions.push_back(pair<int, int>(this->superstring.length() +
                    missing_reads_string.length(), i));
            missing_reads_string += read;
        } else {
            for (int j = 0; j < num_matches; j++) {
                this->positions.push_back(pair<int, int>(indexes[j], i));
            }
            for (int j = 0; j < num_matches2; j++) {
                this->positions.push_back(pair<int, int>(indexes2[j], i));
            }
        }
    }

    cout << "Missing " << missing_read_count << " reads" << endl;
    cout << "Positions size: " << this->positions.size() << endl;

    this->superstring += missing_reads_string;

    int total_reads_size = 0;
    for (int i = 0; i < this->reads.size(); i++) {
        total_reads_size += this->reads[i].length();
    }
    info("Total reads size: " + to_string(total_reads_size) + "\n" +
            "Superstring size:  " + to_string(this->superstring.size()) + "\n" +
            "Compress ratio: " + to_string((float)total_reads_size / (float)this->superstring.size()));

    // rebuild FM-index
    delete fm;
    T = new uint8_t[this->superstring.length() + 1];
    n = this->superstring.length();
    for (int i = 0; i < n; i++) {
        T[i] = this->superstring[i];
    }
    T[n] = '\0';

    debug("Starting building FM-index for the second time ...");
    fm = new FM(T, n, 64);
    if (!fm) {
        throw runtime_error("FM index building failed. Fuck you. That's why.");
    }
    debug("FM index building done. For the second time.");

    string ll;
    while(cin) {
        cout << "waiting for input: " << endl;
        getline(cin, ll);
        boost::algorithm::trim(ll);

        uint8_t* r3 = new uint8_t[ll.length() + 1];
        for (int j = 0; j < ll.length(); j++) {
            r3[j] = ll[j];
        }
        r3[ll.length()] = '\0';

        uint32_t num_matches3;
        uint32_t* indexes3;
        cout << "kokot1" << endl;
        indexes3 = fm->locate(r3, ll.length(), &num_matches3);
        cout << "kokot2" << endl;
        for (int i = 0; i < this->positions.size(); i++) {
            for (int j = 0; j < num_matches3; j++) {
                if (this->positions[i].first == indexes3[j]) {
                    cout << this->positions[i].second << endl;
                }
            }
        }
    };


//    uint8_t* ss = (uint8_t*)"AAATATTTT";
//    int sl = 9;
//    uint32_t cnt;
//    cnt = fm->count(ss, sl);
//    cout << "count of  '" << ss << "' = " << cnt << endl;

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
        cout << msg << endl;
    }
    return;
}

void CR::info(string msg) {
    cout << msg << endl;
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

// TODO: modify this when taking into account reads with uNknowns etc. (R, ... )
bool CR::check_contig(string c) {
    for (int i = 0; i < c.size(); i++) {
        if ((c[i] != 'A') &&
                (c[i] != 'C') &&
                (c[i] != 'T') &&
                (c[i] != 'G')) {
            return false;
        }
    }

    return true;
}

string CR::rev_compl(string s) {
    string rc = "";
    for (int i = 0; i < s.length(); i++) {
        switch (s[i]) {
        case 'A':
            rc += 'T';
            break;
        case 'T':
            rc += 'A';
            break;
        case 'C':
            rc += 'G';
            break;
        case 'G':
            rc += 'C';
            break;
        default:
            rc += s[i];
            break;
        }
    }
    reverse(rc.begin(), rc.end());
    return rc;
}

CR::~CR() {
    //
}
