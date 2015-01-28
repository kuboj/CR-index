#include <string>
#include <vector>
#include <utility>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits>
#include <iostream>
#include <fstream>
#include <tuple>
#include <time.h>
#include <exception>
#include <stdexcept>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
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
                vector<tuple<int, int, char>> diff,
                int read_length = DEFAULT_READ_LENGTH,
                bool verbose = DEFAULT_VERBOSITY);
        vector<int> find_indexes(const string& s);
        vector<string> find_reads(const string& s);
        ~CR();

        static tuple<string, vector<pair<int, int>>, vector<tuple<int, int, char>>>
                     preprocess(string path, bool verbose = DEFAULT_VERBOSITY);

    private:
        int read_length;
        vector<pair<int, int>> positions;
        vector<tuple<int, int, char>> diff;
        FMWrapper fm_index;

        vector<pair<int, int>> locate_positions(const string& s);

        static void debug(string msg);
        static void debug(vector<string> msg);
        static void debug(vector<int> msg);
        static void info(string msg);
};
