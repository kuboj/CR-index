#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

class HashIndex {
    public:
        HashIndex();
        HashIndex(const string& reads_path, int query_length = 13,
                bool save_reads = false, bool verbose = false);
        vector<int> find_indexes(const string& s);
        vector<string> find_reads(const string& s);

    private:
        unordered_map<string, vector<int>> data;
        vector<string> reads;
        int query_length;
        bool save_reads;
        bool verbose;

        bool process(const string& read, int read_id);
        void debug(const string& s);
};
