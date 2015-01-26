#include <string>
#include <vector>
#include <utility>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits>
#include <iostream>
#include <fstream>
#include <time.h>
#include <exception>
#include <stdexcept>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <pstreams/pstream.h>
#include <sdsl/suffix_arrays.hpp>

using namespace sdsl;
using namespace std;

//typedef csa_wt<wt_huff<bit_vector, rank_support_v5<>, select_support_scan<>,
//        select_support_scan<>>, 512, 1024,
//        text_order_sa_sampling<sd_vector<>>> fm_index_type;
typedef  csa_wt<wt_huff<rrr_vector<127> >, 512, 1024> fm_index_type;

class CR {
    public:
        static const bool DEFAULT_VERBOSITY;
        static const int DEFAULT_READ_LENGTH;

        CR(string path, int read_length = DEFAULT_READ_LENGTH,
                bool verbose = DEFAULT_VERBOSITY);
        vector<int> find_indexes(const string& s);
        vector<string> find_reads(const string& s);
        ~CR();

    private:
        bool verbose;
        int read_length;
        vector<pair<int, int>> positions;
        fm_index_type fm_index;

        vector<pair<int, int>> locate_positions(string s);
        void debug(string msg);
        void info(string msg);
        bool check_read(string r);
        bool check_contig(string c);
        string rev_compl(string r);
        string load_contigs(string contigs_path);

        fm_index_type fm_construct(const string& data);
        vector<int> fm_locate(const string &query);
        string fm_extract(int start, int length);
        int fm_memory_size();
};
