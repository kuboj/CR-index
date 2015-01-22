#include "CR.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits>
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
const int CR::DEFAULT_READ_LENGTH = 100;

CR::CR(string path, int read_length, bool verbose) {
    this->read_length = read_length;
    this->verbose = verbose;
    this->positions = vector<pair<int, int>>();

    string superstring = "";
    vector<string >reads = vector<string>();

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
            // TODO: sanitize read - a->A etc
            reads.push_back(l);
        } else {
            skipped++;
        }
    }
    debug("Loaded " + to_string(reads.size()) + " reads. Skipped " +
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
            superstring += l;
        } else {
            skipped++;
        }
    }

    // construct FM-index, find each read in it and add missing reads
    uint8_t* T = new uint8_t[superstring.length() + 1];
    uint32_t n = superstring.length();
    for (int i = 0; i < n; i++) {
        T[i] = superstring[i];
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
    for (int i = 0; i < reads.size(); i++) {
        if (i % 10000 == 0) {
            debug("Processing reads " + to_string(i) + " of " +
                    to_string(reads.size()));
        }
        string read = reads[i];
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
            this->positions.push_back(pair<int, int>(superstring.length() +
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

    superstring += missing_reads_string;

    int total_reads_size = 0;
    for (int i = 0; i < reads.size(); i++) {
        total_reads_size += reads[i].length();
    }
    info("Total reads size: " + to_string(total_reads_size) + "\n" +
            "Superstring size:  " + to_string(superstring.size()) + "\n" +
            "Compress ratio: " + to_string((float)total_reads_size / (float)superstring.size()));

    // rebuild FM-index
    delete fm;
    uint8_t* T2 = new uint8_t[superstring.length() + 1];
    uint32_t n2 = superstring.length();
    for (int i = 0; i < n2; i++) {
        T2[i] = superstring[i];
    }
    T2[n2] = '\0';

    debug("Starting building FM-index for the second time ...");
    this->fm = new FM(T2, n2, 64);
    if (!this->fm) {
        throw runtime_error("FM index building failed. Fuck you. That's why.");
    }
    debug("FM index building done. For the second time.");

    sort(this->positions.begin(), this->positions.end());

    reads.clear();
    superstring.clear();
}

vector<int> CR::locate2(string s) {
    vector<int> retval = vector<int>();

    uint8_t* r = new uint8_t[s.length() + 1];
    for (int i = 0; i < s.length(); i++) {
        r[i] = s[i];
    }
    r[s.length()] = '\0';
    uint32_t num_matches;
    uint32_t* indexes;
    indexes = this->fm->locate(r, s.length(), &num_matches);

    for (int i = 0; i < num_matches; i++) {
        pair<int, int> start_index(indexes[i] + s.length() - this->read_length, -1);
        pair<int, int> end_index(indexes[i], numeric_limits<int>::max());
        auto low = lower_bound(this->positions.begin(), this->positions.end(), start_index);
        auto up = upper_bound(this->positions.begin(), this->positions.end(), end_index);
        for (auto it = low; it != up; it++) {
            retval.push_back(it->second);
        }
    }

    return retval;
}

vector<int> CR::locate(string s) {
    boost::algorithm::trim(s);
    vector<int> retval = this->locate2(s);
    for (int i : this->locate2(rev_compl(s))) {
        retval.push_back(i);
    }

    sort(retval.begin(), retval.end());

    string debug_string = "";
    for (auto i : retval) {
        debug_string += to_string(i) + ", ";
    }
    debug(debug_string);

    return retval;
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
