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
#include "fm_wrapper.hpp"
#include "util.hpp"

using namespace std;

class CR {
    public:
        static const bool DEFAULT_VERBOSITY;
        static const int DEFAULT_READ_LENGTH;
        static bool verbose;

        CR(string path, int read_length = DEFAULT_READ_LENGTH,
                bool verbose = DEFAULT_VERBOSITY);
        CR(string superstring, vector<pair<int, int>> positions,
                int read_length = DEFAULT_READ_LENGTH,
                bool verbose = DEFAULT_VERBOSITY);
        vector<int> find_indexes(const string& s);
        vector<string> find_reads(const string& s);
        ~CR();

        static pair<string, vector<pair<int, int>>> preprocess(string path);

    private:
        int read_length;
        vector<pair<int, int>> positions;
        FMWrapper fm_index;

        vector<pair<int, int>> locate_positions(string s);

        static void debug(string msg);
        static void info(string msg);
};
