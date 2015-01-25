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

    boost::filesystem::path genome_path = boost::filesystem::path(path);

    ifstream f(genome_path.string().c_str());

    if (!f) {
        throw runtime_error("Error opening file '" +
                genome_path.string()+ "'");
    }

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
        debug(s);
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
        debug(s2);
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
        debug(s3);
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
    string superstring = load_contigs(contigs_path.string());

    // construct FM-index, find each read in it and add missing reads
    this->fm_index = fm_construct(superstring);

    int missing_read_count = 0;
    int total_reads_size = 0;

    debug("Querying FM index for reads");
    string l;
    int read_count = 0;
    int line_count = 0;
    int skipped = 0;
    while (getline(f, l)) {
        if (line_count++ % 4 != 1) {
            continue;
        }
        boost::algorithm::trim(l);
        if (!check_read(l)) {
            skipped++;
            continue;
        }

        total_reads_size += l.length();

        if (read_count % 10000 == 0) {
            debug("Processed " + to_string(read_count) + " reads");
        }

        vector<int> matches = fm_locate(l);
        vector<int> matches2 = fm_locate(rev_compl(l));

        if (matches.size() == 0 && matches2.size() == 0) {
            missing_read_count++;
            this->positions.push_back(pair<int, int>(superstring.length(),
                    read_count));
            superstring += l;
        } else {
            for (int m : matches) {
                this->positions.push_back(pair<int, int>(m, read_count));
            }
            for (int m : matches2) {
                this->positions.push_back(pair<int, int>(m, read_count));
            }
        }

        read_count++;
    }

    debug("Missing " + to_string(missing_read_count) + " reads");
    debug("Positions size: " + to_string(this->positions.size()));
    info("Total reads size: " + to_string(total_reads_size) + "\n" +
            "Superstring size:  " + to_string(superstring.size()) + "\n" +
            "Compress ratio: " + to_string((float)total_reads_size / (float)superstring.size()));

    // rebuild FM-index
    delete this->fm_index;
    this->fm_index = fm_construct(superstring);

    sort(this->positions.begin(), this->positions.end());

    superstring.clear();
}

FM* CR::fm_construct(string s) {
    uint8_t* T = new uint8_t[s.length() + 1];
    uint32_t n = s.length();

    for (int i = 0; i < n; i++) {
        T[i] = s[i];
    }
    T[n] = '\0';

    debug("Starting building FM-index");
    FM* fm = new FM(T, n + 1, 64);
    if (!fm) {
        throw runtime_error("FM index building failed. Fuck you. That's why.");
    }
    debug("FM index building done.");

    return fm;
}

vector<int> CR::fm_locate(string p) {
    vector<int> retval;
    uint8_t* r = new uint8_t[p.length() + 1];
    for (int i = 0; i < p.length(); i++) {
        r[i] = p[i];
    }
    r[p.length()] = '\0';

    uint32_t num_matches;
    uint32_t* indexes;
    indexes = this->fm_index->locate(r, p.length(), &num_matches);
    for (int i = 0; i < num_matches; i++) {
        retval.push_back(indexes[i]);
    }

    delete[] r;
    delete[] indexes;

    return retval;
}

string CR::fm_extract(int start, int length) {
    string retval = "";
    cout << "fm extract " << start << " " << length << endl;
    uint8_t* s = this->fm_index->extract(start - 1, start - 1 + this->read_length);
    for (int i = 0; i < length; i++) {
        retval += s[i];
    }
    cout << "fm extract done" << endl;

    delete[] s;
    cout << retval << endl;
    return retval;
}

vector<pair<int, int>> CR::locate_positions(string s) {
    vector<pair<int, int>> retval;
    vector<int> indexes = fm_locate(s);

    for (auto i : indexes) {
        pair<int, int> start_index(i + s.length() - this->read_length, -1);
        pair<int, int> end_index(i, numeric_limits<int>::max());
        auto low = lower_bound(this->positions.begin(), this->positions.end(), start_index);
        auto up = upper_bound(this->positions.begin(), this->positions.end(), end_index);
        for (auto it = low; it != up; it++) {
            retval.push_back(*it);
        }
    }

    return retval;
}

vector<int> CR::locate(string s) {
    boost::algorithm::trim(s);
    vector<int> retval;

    for (auto i : this->locate_positions(s)) {
        retval.push_back(i.second);
    }
    for (auto i : this->locate_positions(rev_compl(s))) {
        retval.push_back(i.second);
    }

    sort(retval.begin(), retval.end());

    string debug_string = "";
    for (auto i : retval) {
        debug_string += to_string(i) + ", ";
    }
    debug(debug_string);

    return retval;
}

vector<string> CR::locate2(string s) {
    boost::algorithm::trim(s);
    vector<string> retval;

    for (auto i : this->locate_positions(s)) {
        cout << i.second << endl;
        retval.push_back(fm_extract(i.first, this->read_length));
    }
    for (auto i : this->locate_positions(rev_compl(s))) {
        cout << i.second << endl;
        retval.push_back(fm_extract(i.first, this->read_length));
    }

    //sort(retval.begin(), retval.end());

    string debug_string = "";
    for (auto i : retval) {
        debug_string += i + '\n';
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

string CR::load_contigs(string contigs_path) {
    string retval = "";
    ifstream f(contigs_path);

    if (!f) {
        throw runtime_error("Error opening file '" + contigs_path + "'");
    }

    string l = "";
    int i = 0;
    int skipped = 0;
    while (getline(f, l)) {
        if (i++ % 2 != 1) {
            continue;
        }
        boost::algorithm::trim(l);
        if (check_contig(l)) {
            retval += l;
        } else {
            skipped++;
        }
    }

    debug("Loaded " + to_string(i / 2) + " contigs. " + to_string(skipped) +
            " skipped");

    f.close();

    return retval;
}

CR::~CR() {
    //
}
