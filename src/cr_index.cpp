#include "cr_index.hpp"

using namespace std;

const bool CRIndex::DEFAULT_VERBOSITY = false;
const int CRIndex::DEFAULT_READ_LENGTH = 100;
bool CRIndex::verbose = CRIndex::DEFAULT_VERBOSITY;

CRIndex::CRIndex(string p, int rl, bool v) {
    this->positions = vector<t_pos>();
    this->diff = vector<t_diff>();
    this->read_length = rl;
    CRIndex::verbose = v;

    string superstring;

    tie(superstring, this->positions, this->diff) = preprocess(p, v);
    this->fm_index = FMWrapper(superstring);
    sort(this->positions.begin(), this->positions.end());
    superstring.clear();
}

CRIndex::CRIndex(string superstring, vector<t_pos> p, vector<t_diff> d, int rl, bool v) {
    this->positions = p;
    this->diff = d;
    this->read_length = rl;
    CRIndex::verbose = v;

    this->fm_index = FMWrapper(superstring);
    sort(this->positions.begin(), this->positions.end());
    superstring.clear();
}

tuple<string, vector<t_pos>, vector<t_diff>> CRIndex::preprocess(string p, bool v) {
    CRIndex::verbose = v;
    vector<t_pos> _positions = vector<t_pos>();
    vector<t_diff> _diff = vector<t_diff>();

    boost::filesystem::path orig_reads_path = boost::filesystem::path(p);

    // create temporary directory
    boost::filesystem::path tmpdir = cr_util::create_tmpdir();
    debug("Temporary directory " + tmpdir.string() + " created");
    boost::filesystem::path p_tmpdir = tmpdir / "cr"; // e.g. /tmp/634634/cr

    debug("Running sga index");
    debug(cr_util::execute_command("sga index -a ropebwt -c -t 4 -p " +
            p_tmpdir.string() + " " + orig_reads_path.string()));
    debug("Running sga correct");
    debug(cr_util::execute_command("sga correct -t 4 -p " +
                p_tmpdir.string() + " -o " + p_tmpdir.string() + ".ec.fa " +
                orig_reads_path.string()));
    boost::filesystem::path corr_reads_path = boost::filesystem::path(
            p_tmpdir.string() + ".ec.fa");
    cr_util::check_path_existence(corr_reads_path);

    boost::filesystem::path ncrit_reads_path = boost::filesystem::path(
                p_tmpdir.string() + ".ncrit");

    ifstream orig_reads_istream(orig_reads_path.string());
    ifstream corr_reads_istream(corr_reads_path.string());
    ofstream ncrit_reads_ostream(ncrit_reads_path.string());

    string orig_read;
    string corr_read;
    string read_label;
    string read_meta;
    string read_q;
    string ll;
    int crit_count = 0;
    int ncrit_count = 0;
    int read_count = 0;
    debug("Searching for critical reads");
    while (getline(orig_reads_istream, read_label)) {
        getline(corr_reads_istream, ll);
        getline(corr_reads_istream, corr_read);
        getline(corr_reads_istream, ll);
        getline(corr_reads_istream, ll);

        getline(orig_reads_istream, orig_read);
        getline(orig_reads_istream, read_meta);
        getline(orig_reads_istream, read_q);

        boost::algorithm::trim(orig_read);
        boost::algorithm::trim(corr_read);
        vector<int> diff_indexes = cr_util::diff_indexes(orig_read, corr_read);
        if (diff_indexes.size() >= 2 && cr_util::indexes_close(diff_indexes, 15)) {
            int read_id = read_count;
            for (int i : diff_indexes) {
                _diff.push_back(make_tuple(read_id, i, orig_read[i]));
            }
            crit_count += 1;
        } else {
            ncrit_reads_ostream << read_label << endl;
            ncrit_reads_ostream << corr_read << endl;
            ncrit_reads_ostream << read_meta << endl;
            ncrit_reads_ostream << read_q << endl;
            int read_id = read_count;
            for (int i : diff_indexes) {
                _diff.push_back(make_tuple(read_id, i, orig_read[i]));
            }
            ncrit_count += 1;
        }

        read_count++;
    }
    ncrit_reads_ostream.close();
    orig_reads_istream.close();
    // rewind
    corr_reads_istream.clear();
    corr_reads_istream.seekg(0);
    debug("Total " + to_string(read_count) + " reads. critical: " +
            to_string(crit_count) + " noncritical: " + to_string(ncrit_count));

    // rerun sga index with noncritical reads only
    debug("Running sga index");
    debug(cr_util::execute_command("sga index -a ropebwt -c -t 4 -p " +
            p_tmpdir.string() + " " + ncrit_reads_path.string()));
    debug("Running sga overlap");
    debug(cr_util::execute_command("sga overlap -t 4 -p " +
            p_tmpdir.string() + " " + ncrit_reads_path.string()));

    // get path of overlap file because dickish sga overlap saves it to workdir
    boost::filesystem::path overlap_path = boost::filesystem::path(
            boost::filesystem::current_path() / (ncrit_reads_path.stem().string() + ".asqg.gz"));
    cr_util::check_path_existence(overlap_path);

    // move overlap file to tmpdir
    boost::filesystem::path aux = boost::filesystem::path(p_tmpdir.string() + ".asqg.gz");
    boost::filesystem::rename(overlap_path, aux);
    overlap_path = aux;

    debug("Running sga assemble");
    debug(cr_util::execute_command("sga assemble -o " + p_tmpdir.string() +
            " " + overlap_path.string()));

    boost::filesystem::path contigs_path = boost::filesystem::path(
            p_tmpdir.string() + "-contigs.fa");
    cr_util::check_path_existence(contigs_path);

    // read contigs
    string superstring = cr_util::load_contigs(contigs_path.string());

    // construct FM-index, find each read in it and add missing reads
    FMWrapper _fm_index = FMWrapper(superstring);

    debug("Querying FM index for reads");
    int missing_read_count = 0;
    int total_reads_size = 0;
    string read;
    read_count = 0;
    int line_count = 0;
    int skipped = 0;
    while (getline(corr_reads_istream, read)) {
        if (line_count++ % 4 != 1) {
            continue;
        }
        boost::algorithm::trim(read);
        if (!cr_util::check_read(read)) {
            skipped++;
            continue;
        }

        total_reads_size += read.length();

        if (read_count % 10000 == 0) {
            debug("Processed " + to_string(read_count) + " reads");
        }

        vector<int> matches = _fm_index.locate(read);
        vector<int> matches2 = _fm_index.locate(cr_util::rev_compl(read));

        if (matches.size() == 0 && matches2.size() == 0) {
            missing_read_count++;
            _positions.push_back(make_tuple(superstring.length(), read_count, 0));
            superstring += read;
        } else {
            for (int m : matches) {
                _positions.push_back(make_tuple(m, read_count, 0));
                if (read_count == 581) {
                    cout << "581 " << m << endl;
                }
            }
            for (int m : matches2) {
                _positions.push_back(make_tuple(m, read_count, 1));
                if (read_count == 581) {
                    cout << "581 " << m << " revcompl" << endl;
                }
            }
        }

        read_count++;
    }
    corr_reads_istream.close();

    if (skipped > 0) {
        info("WARNING: skipped " + to_string(skipped) + " reads");
    }
    debug("Processed " + to_string(read_count) + " reads");
    debug("Missing " + to_string(missing_read_count) + " reads (noncritical " +
            "dropped by sga assembler + critical)");
    debug("Positions size: " + to_string(_positions.size()));
    info("Total reads size: " + to_string(total_reads_size) + "\n" +
            "Superstring size:  " + to_string(superstring.size()) + "\n" +
            "Compress ratio: " + to_string((float)total_reads_size / (float)superstring.size()));

    return make_tuple(superstring, _positions, _diff);
}

vector<t_pos> CRIndex::locate_positions2(const string& s, const string& s_check) {
    vector<t_pos> retval;
    vector<int> indexes = this->fm_index.locate(s);

    for (auto i : indexes) {
        t_pos start_index(i + s.length() - this->read_length, -1, 0);
        t_pos end_index(i, numeric_limits<int>::max(), 1);
        auto low = lower_bound(this->positions.begin(), this->positions.end(), start_index);
        auto up = upper_bound(this->positions.begin(), this->positions.end(), end_index);
        for (auto it = low; it != up; it++) {
            // drop false positives
            bool rev_compl = get<2>(*it);
            string orig_read = extract_original_read(*it);
            string p = s_check;
            if (rev_compl) {
                p = cr_util::rev_compl(s_check);
            }
            if (orig_read.find(p) != string::npos) {
                retval.push_back(*it);
            }
        }
    }

    return retval;
}

vector<t_pos> CRIndex::locate_positions(const string& s) {
    vector<t_pos> retval = locate_positions2(s, s);

    for (string s2 : cr_util::strings_with_edt1(s)) {
        vector<t_pos> temp = locate_positions2(s2, s);
        retval.insert(retval.end(), temp.begin(), temp.end());
    }

    return retval;
}

vector<int> CRIndex::find_indexes(const string& s) {
    boost::algorithm::trim_copy(s);
    vector<int> retval;

    for (auto i : this->locate_positions(s)) {
        retval.push_back(get<1>(i));
    }
    for (auto i : this->locate_positions(cr_util::rev_compl(s))) {
        retval.push_back(get<1>(i));
    }

    sort(retval.begin(), retval.end());
    retval.erase(unique(retval.begin(), retval.end()), retval.end());

    debug("Found " + to_string(retval.size()) + " occurrences.");
    debug(retval);

    return retval;
}

vector<string> CRIndex::find_reads(const string& s) {
    boost::algorithm::trim_copy(s);
    vector<string> retval;

    for (auto i : this->locate_positions(s)) {
        retval.push_back(extract_original_read(i));
    }
    for (auto i : this->locate_positions(cr_util::rev_compl(s))) {
        retval.push_back(extract_original_read(i));
    }

    sort(retval.begin(), retval.end());
    retval.erase(unique(retval.begin(), retval.end()), retval.end());

    debug("Found " + to_string(retval.size()) + " occurrences.");
    debug(retval);

    return retval;
}

string CRIndex::extract_original_read(t_pos read_p) {
    int pos = get<0>(read_p);
    int read_id = get<1>(read_p);
    bool rev_compl = get<2>(read_p);

    t_diff start_index(read_id, -1, 'A');
    t_diff end_index(read_id, numeric_limits<int>::max(), 'A');
    auto low = lower_bound(this->diff.begin(), this->diff.end(), start_index);
    auto up = upper_bound(this->diff.begin(), this->diff.end(), end_index);

    string retval = this->fm_index.extract(pos, this->read_length);
    if (rev_compl) {
        retval = cr_util::rev_compl(retval);
    }
    for (auto it = low; it != up; it++) {
        retval[get<1>(*it)] = get<2>(*it);
    }

    return retval;
}

void CRIndex::debug(string msg) {
    if (!CRIndex::verbose) {
        return;
    }

    cout << msg << endl;

    return;
}

void CRIndex::debug(vector<string> msg) {
    if (!CRIndex::verbose) {
        return;
    }

    string debug_string = "";
    for (auto i : msg) {
        debug_string += i + '\n';
    }
    cout << debug_string << endl;

    return;
}

void CRIndex::debug(vector<int> msg) {
    if (!CRIndex::verbose) {
        return;
    }

    string debug_string = "";
    for (auto i : msg) {
        debug_string += to_string(i) + ", ";
    }
    cout << debug_string << endl;

    return;
}


void CRIndex::info(string msg) {
    cout << msg << endl;
    return;
}

CRIndex::~CRIndex() {
    //
}
