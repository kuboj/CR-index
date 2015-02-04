#include "hash_index.hpp"
#include "util.hpp"

using namespace std;

HashIndex::HashIndex() {
    //
}

HashIndex::HashIndex(string p, int q, bool sr) {
    this->data = unordered_map<string, vector<int>>();
    this->reads = vector<string>();
    this->query_length = q;
    this->save_reads = sr;

    cout << "Constructing HashIndex ..." << endl;

    ifstream f(p);
    if (!f) {
        cerr << "Error opening " << p << endl;
        exit(1);
    }

    string l;
    int i = -1;
    int reads_count = 0;
    while (getline(f, l)) {
        i++;
        if (i % 4 != 1) {
            continue;
        }
        if ((i / 4) % 10000 == 0) {
            cout << "processed " << reads_count << " reads" << endl;
        }
        if (process(l, i / 4)) {
            reads_count++;
        }

        if (this->save_reads) {
            this->reads.push_back(l);
        }
    }
    cout << "processed " << reads_count << " reads" << endl;
}

vector<int> HashIndex::find_indexes(const string& s) {
    vector<int> retval;
    for (int i : this->data[s]) {
        retval.push_back(i);
    }
    for (int i : this->data[cr_util::rev_compl(s)]) {
        retval.push_back(i);
    }
    sort(retval.begin(), retval.end());
    retval.erase(unique(retval.begin(), retval.end()), retval.end());

    return retval;
}

vector<string> HashIndex::find_reads(const string& s) {
    if (!this->save_reads) {
        throw runtime_error("save_reads must be enabled");
    }

    vector<string> retval;
    for (int i : find_indexes(s)) {
        retval.push_back(this->reads[i]);
    }
    for (int i : find_indexes(cr_util::rev_compl(s))) {
        retval.push_back(this->reads[i]);
    }
    sort(retval.begin(), retval.end());
    retval.erase(unique(retval.begin(), retval.end()), retval.end());

    return retval;
}

bool HashIndex::process(const string& read, int read_id) {
    for (size_t i = 0; i < read.length() - this->query_length + 1; i++) {
        string substr = read.substr(i, this->query_length);
        if (this->data.find(substr) == this->data.end()) {
            this->data[substr] = vector<int>();
        }
        this->data[substr].push_back(read_id);
    }

    return true;
}
